#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "safeguards.h"
#include "pool.h"
#include "freelist.h"

int freelist_alloc(freelist_t *free_list, unsigned long int item_count, unsigned long int item_size)
{
#ifdef ERROR_CHECKING
    if (free_list == NULL)
        THROW_ERR(-1, "NULL FREELIST PTR", return retval);
#endif

    CHECK(pool_alloc(&(free_list->pool), item_count, item_size), return retval);

    THROW_ERR(pthread_mutex_lock(&(free_list->mutex_lock)), strerror(errno), return retval);
    free_list->first = 0;
    THROW_ERR(pthread_mutex_unlock(&(free_list->mutex_lock)), strerror(errno), return retval);

    for (unsigned long int i = 0; i < free_list->pool.max_count; i++)
    {
        unsigned long int next = i + 1ul;
        CHECK(pool_cpy_in_b(&(free_list->pool), &next, i, sizeof(unsigned long int)), return retval);
    }

    return 0;
}

int freelist_free(freelist_t *free_list)
{
#ifdef ERROR_CHECKING
    if (free_list == NULL)
        THROW_ERR(-1, "NULL FREELIST PTR", return retval);
#endif

    THROW_ERR(pthread_mutex_lock(&(free_list->mutex_lock)), strerror(errno), return retval);
    free_list->first = -1;
    THROW_ERR(pthread_mutex_unlock(&(free_list->mutex_lock)), strerror(errno), return retval);
    CHECK(pool_free(&(free_list->pool)), return retval);
    return 0;
}

int freelist_aquire(unsigned long int *dst_index_ptr, freelist_t *free_list)
{
#ifdef ERROR_CHECKING
    if (dst_index_ptr == NULL)
        THROW_ERR(-1, "NULL dst_index_ptr", return retval);
    if (free_list == NULL)
        THROW_ERR(-1, "NULL FREELIST PTR", return retval);
#endif

    THROW_ERR(pthread_mutex_lock(&(free_list->mutex_lock)), strerror(errno), return retval);

    *dst_index_ptr = free_list->first;

    CHECK(pool_cpy_out_b(&(free_list->pool), &(free_list->first), free_list->first, sizeof(unsigned long int)), {
        THROW_ERR(pthread_mutex_unlock(&(free_list->mutex_lock)), strerror(errno), return retval);
        return retval;
    });

    THROW_ERR(pthread_mutex_unlock(&(free_list->mutex_lock)), strerror(errno), return retval);

    return 0;
}

int freelist_release(unsigned long int *src_index_ptr, freelist_t *free_list)
{
#ifdef ERROR_CHECKING
    if (src_index_ptr == NULL)
        THROW_ERR(-1, "NULL src_index_ptr PTR", return retval);
    if (free_list == NULL)
        THROW_ERR(-1, "NULL FREELIST PTR", return retval);
#endif
    THROW_ERR(pthread_mutex_lock(&(free_list->mutex_lock)), strerror(errno), return retval);

    CHECK(pool_cpy_in_b(&(free_list->pool), &(free_list->first), *src_index_ptr, sizeof(unsigned long int)), {
        THROW_ERR(pthread_mutex_unlock(&(free_list->mutex_lock)), strerror(errno), return retval);
        return retval;
    });

    free_list->first = *src_index_ptr;
    *src_index_ptr = -1;

    THROW_ERR(pthread_mutex_unlock(&(free_list->mutex_lock)), strerror(errno), return retval);

    return 0;
}