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

    free_list->first = 0;
    memset(free_list->pool.block, -1, free_list->pool.block_size);

    for (unsigned long int i = 0; i < free_list->pool.max_count; i++)
    {
        unsigned long int *pool_ptr = NULL;
        CHECK(pool_get_ptr((void **)&pool_ptr, &(free_list->pool), i), return retval);
        *pool_ptr = i + 1ul;
    }
    return 0;
}

int freelist_free(freelist_t *free_list)
{
#ifdef ERROR_CHECKING
    if (free_list == NULL)
        THROW_ERR(-1, "NULL FREELIST PTR", return retval);
#endif

    free_list->first = -1;
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

    unsigned long int *first_pool_ptr = NULL;
    CHECK(pool_get_ptr((void **)&first_pool_ptr, &(free_list->pool), free_list->first), return retval);

#ifdef ERROR_CHECKING
    if (first_pool_ptr == NULL)
        THROW_ERR(-1, "NULL first_pool_ptr PTR", return retval);
#endif

    *dst_index_ptr = free_list->first;
    free_list->first = *first_pool_ptr;
    *first_pool_ptr = -1;

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

    unsigned long int *src_pool_ptr = NULL;
    CHECK(pool_get_ptr((void **)&src_pool_ptr, &(free_list->pool), *src_index_ptr), return retval);

#ifdef ERROR_CHECKING
    if (src_pool_ptr == NULL)
        THROW_ERR(-1, "NULL tmp_ptr PTR", return retval);
#endif

    *src_pool_ptr = free_list->first;
    free_list->first = *src_index_ptr;
    *src_index_ptr = -1;

    return 0;
}