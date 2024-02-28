
#include <string.h>
#include <stddef.h>
#include <errno.h>

#include "util.h"
#include "swapchain.h"
#include "safeguards.h"

int __swapchain_read_close(swapchain_t *chain, swaplink_t *prev_link)
{
    CHECK_ERR(pthread_rwlock_unlock(&(prev_link->data_rwlock)), strerror(errno), return errno);
    CHECK_ERR(pthread_cond_broadcast(&(chain->done_read_cond)), strerror(errno), return errno);
    return 0;
}

int __swapchain_read_front(swapchain_t *chain, swaplink_t **link_ptr, const void **data)
{
    swaplink_t *prev_link = *link_ptr;

    if (prev_link != NULL)
    {
        CHECK_ERR(__swapchain_read_close(chain, prev_link), strerror(errno), return errno);
    }

    CHECK_ERR(pthread_rwlock_rdlock(&(chain->newest_rwlock)), strerror(errno), return errno);
    CHECK_ERR(pthread_rwlock_rdlock(&(chain->newest->data_rwlock)), strerror(errno), return errno);
    *link_ptr = chain->newest;
    *data = chain->newest->data;
    CHECK_ERR(pthread_rwlock_unlock(&(chain->newest_rwlock)), strerror(errno), return errno);
    return 0;
}

int __swapchain_read_next(swapchain_t *chain, swaplink_t **link_ptr, const void **data)
{
    swaplink_t *prev_link = *link_ptr;
    if (prev_link == NULL)
    {
        return __swapchain_read_front(chain, link_ptr, data);
    }
    CHECK_ERR(__swapchain_read_close(chain, prev_link), strerror(errno), return errno);

    CHECK_ERR(pthread_mutex_lock(&(prev_link->newer_rwlock)), strerror(errno), return errno);
    CHECK_ERR(pthread_rwlock_rdlock(&(prev_link->newer->data_rwlock)), strerror(errno), return errno);
    *link_ptr = prev_link->newer;
    *data = prev_link->newer->data;
    CHECK_ERR(pthread_mutex_unlock(&(prev_link->newer_rwlock)), strerror(errno), return errno);
    return 0;
}

int __swapchain_write_close(swapchain_t *chain, swaplink_t *prev_link)
{
    CHECK_ERR(pthread_rwlock_unlock(&(prev_link->data_rwlock)), strerror(errno), return errno);

    CHECK_ERR(pthread_rwlock_wrlock(&(chain->newest_rwlock)), strerror(errno), return errno);
    if (chain->newest != NULL)
    {
        CHECK_ERR(pthread_mutex_lock(&(chain->newest->newer_rwlock)), strerror(errno), return errno);
        chain->newest->newer = prev_link;
        CHECK_ERR(pthread_mutex_unlock(&(chain->newest->newer_rwlock)), strerror(errno), return errno);
    }
    chain->newest = prev_link;
    CHECK_ERR(pthread_rwlock_unlock(&(chain->newest_rwlock)), strerror(errno), return errno);

    CHECK_ERR(pthread_mutex_lock(&(chain->oldest_rwlock)), strerror(errno), return errno);
    if (chain->oldest == NULL)
    {
        chain->oldest = prev_link;
    }
    CHECK_ERR(pthread_mutex_unlock(&(chain->oldest_rwlock)), strerror(errno), return errno);
    return 0;
}

int __swapchain_write_back(swapchain_t *chain, swaplink_t **link_ptr, const void **data)
{
    swaplink_t *older_link = NULL;
    swaplink_t *newer_link = NULL;

    if (*link_ptr != NULL)
    {
        CHECK_ERR(__swapchain_write_close(chain, *link_ptr), strerror(errno), return errno);
    }

    CHECK_ERR(pthread_mutex_lock(&(chain->oldest_rwlock)), strerror(errno), return errno);
    newer_link = chain->oldest;
    int retval = pthread_rwlock_trywrlock(&(newer_link->data_rwlock));
    while (retval == EBUSY)
    {
        if (older_link != NULL)
        {
            CHECK_ERR(pthread_mutex_unlock(&(older_link->newer_rwlock)), strerror(errno), return errno);
        }
        older_link = newer_link;
        CHECK_ERR(pthread_mutex_lock(&(older_link->newer_rwlock)), strerror(errno), return errno);
        newer_link = older_link->newer;
        if (newer_link != NULL)
        {
            CHECK_ERR(pthread_mutex_unlock(&(older_link->newer_rwlock)), strerror(errno), return errno);
            newer_link = chain->oldest;
            older_link = NULL;
            CHECK_ERR(pthread_cond_wait(&(chain->done_read_cond), &(chain->oldest_rwlock)), strerror(errno), return errno);
        }
        retval = pthread_rwlock_trywrlock(&(newer_link->data_rwlock));
    }
    CHECK_ERR(retval, strerror(errno), return errno);

    CHECK_ERR(pthread_mutex_lock(&(newer_link->newer_rwlock)), strerror(errno), return errno);
    if (older_link != NULL)
    {
        older_link->newer = newer_link->newer;
        CHECK_ERR(pthread_mutex_unlock(&(older_link->newer_rwlock)), strerror(errno), return errno);
    }
    else
    {
        chain->oldest = newer_link->newer;
    }
    CHECK_ERR(pthread_mutex_unlock(&(chain->oldest_rwlock)), strerror(errno), return errno);
    newer_link->newer = NULL;
    CHECK_ERR(pthread_mutex_unlock(&(newer_link->newer_rwlock)), strerror(errno), return errno);
    *link_ptr = newer_link;
    *data = newer_link->data;

    return 0;
}

#define DEFAULT_SWAPCHAIN_READ_CLOSE __swapchain_read_close
#define DEFAULT_SWAPCHAIN_READ_FRONT __swapchain_read_front
#define DEFAULT_SWAPCHAIN_READ_NEXT __swapchain_read_next
#define DEFAULT_SWAPCHAIN_WRITE_CLOSE __swapchain_write_close
#define DEFAULT_SWAPCHAIN_WRITE_BACK __swapchain_write_back

int swapchain_init(
    swapchain_t *chain,
    size_t backing_buffer_length,
    void *backing_buffer,
    size_t item_size,
    size_t chunk_alignment,
    swapchain_read_close_func_t read_close_fun,
    swapchain_read_front_func_t read_front_fun,
    swapchain_read_next_func_t read_next_fun,
    swapchain_write_close_func_t write_close_fun,
    swapchain_write_back_func_t write_back_fun)
{
    item_size = align_forward(item_size, chunk_alignment);
    swaplink_t *link = (swaplink_t *)align_forward((unsigned long)backing_buffer, chunk_alignment);
    chain->oldest = link;
    do
    {
        link->newer = (swaplink_t *)align_forward(((size_t)link + item_size + sizeof(swaplink_t)), chunk_alignment);
        if ((size_t)(link->newer) >= (size_t)backing_buffer + backing_buffer_length)
        {
            chain->newest = link;
            link->newer = NULL;
        }
        link = link->newer;
    } while (link != NULL);

    chain->read_close = read_close_fun == NULL ? DEFAULT_SWAPCHAIN_READ_CLOSE : read_close_fun;
    chain->read_front = read_front_fun == NULL ? DEFAULT_SWAPCHAIN_READ_FRONT : read_front_fun;
    chain->read_next = read_next_fun == NULL ? DEFAULT_SWAPCHAIN_READ_NEXT : read_next_fun;
    chain->write_close = write_close_fun == NULL ? DEFAULT_SWAPCHAIN_WRITE_CLOSE : write_close_fun;
    chain->write_back = write_back_fun == NULL ? DEFAULT_SWAPCHAIN_WRITE_BACK : write_back_fun;
    return 0;
}