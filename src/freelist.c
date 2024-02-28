#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "util.h"
#include "safeguards.h"
#include "freelist.h"

int __freelist_clr(freelist_t *fl, size_t item_size)
{
    CHECK_ERR(fl == NULL || item_size <= 0 ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);

    void **item = fl->start;

    unsigned long total_count = 0;
    while ((size_t)item + item_size < (size_t)fl->start + fl->buf_len)
    {
        total_count++;
        *item = (void *)(((size_t)item) + item_size);
        item = *item;
    }
    total_count = 0;
    return 0;
}

int __freelist_get(freelist_t *fl, void **item)
{
    CHECK_ERR(fl == NULL || item == NULL ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);

    CHECK_ERR(pthread_mutex_lock(&(fl->lock_list)), strerror(errno), return errno);

    if (fl->first != NULL)
    {
        void **free = fl->first;
        fl->first = *free;
        *item = free;
    }

    CHECK_ERR(pthread_mutex_unlock(&(fl->lock_list)), strerror(errno), return errno);
    return 0;
}

int __freelist_rel(freelist_t *fl, void *item)
{
    CHECK_ERR(fl == NULL || item < fl->start || item >= (void *)((size_t)fl->start + fl->buf_len) ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);

    CHECK_ERR(pthread_mutex_lock(&(fl->lock_list)), strerror(errno), return errno);
    void **free = item;
    *free = fl->first;
    fl->first = item;
    CHECK_ERR(pthread_mutex_unlock(&(fl->lock_list)), strerror(errno), return errno);
    return 0;
}

#define DEFAULT_FREELIST_GET __freelist_get
#define DEFAULT_FREELIST_REL __freelist_rel

int freelist_init(freelist_t *fl,
                  size_t backing_buffer_length,
                  void *backing_buffer,
                  size_t item_size,
                  size_t chunk_alignment,
                  freelist_get_func_t get_func,
                  freelist_rel_func_t rel_func)
{
    CHECK_ERR(fl == NULL || backing_buffer == NULL || item_size <= 0 ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);

    memset(backing_buffer, 0, backing_buffer_length);

    // Align backing buffer to the specified chunk alignment
    void *start = (void *)align_forward((unsigned long)backing_buffer, chunk_alignment);

    backing_buffer_length -= start - backing_buffer;

    // Align chunk size up to the required chunk_alignment
    item_size = align_forward(item_size, chunk_alignment);

    // Assert that the parameters passed are valid
    CHECK_ERR(backing_buffer_length < item_size ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);

    // Store the adjusted parameters
    fl->buf = (unsigned char *)backing_buffer;
    fl->buf_len = backing_buffer_length;
    fl->item_size = item_size;

    fl->start = start;
    fl->first = start;

    CHECK_ERR(__freelist_clr(fl, item_size), strerror(errno), return errno);

    fl->get = get_func == NULL ? DEFAULT_FREELIST_GET : get_func;
    fl->rel = rel_func == NULL ? DEFAULT_FREELIST_REL : rel_func;

    return 0;
}
