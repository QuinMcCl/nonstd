#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "safeguards.h"
#include "arraylist.h"

int __arraylist_push_back(arraylist_t *arraylist, void *src, size_t item_count, size_t item_size, size_t item_stride)
{
    CHECK_ERR(
        arraylist == NULL
            ? EINVAL
            : EXIT_SUCCESS,
        strerror(errno), return errno);

    if (item_count == 0)
    {
        return 0;
    }

    int retval = 0;
    CHECK_ERR(pthread_rwlock_wrlock(&(arraylist->rwlock)), strerror(errno), return errno);
    if (arraylist->item_count + item_count > arraylist->max_count)
    {
        retval = EINVAL;
    }
    if (retval == 0)
    {
        retval = POOL_CPY_IN(arraylist->pool, src, arraylist->item_count, item_count, item_size, item_stride);
    }
    if (retval == 0)
    {
        arraylist->item_count += item_count;
    }
    CHECK_ERR(pthread_rwlock_unlock(&(arraylist->rwlock)), strerror(errno), return errno);
    CHECK_ERR(retval, strerror(errno), return errno);
    return retval;
}
int __arraylist_pop_back(arraylist_t *arraylist, void *dst, size_t item_count, size_t item_size, size_t item_stride)
{
    CHECK_ERR(
        arraylist == NULL
            ? EINVAL
            : EXIT_SUCCESS,
        strerror(errno), return errno);

    if (item_count == 0)
    {
        return 0;
    }

    int retval = 0;
    CHECK_ERR(pthread_rwlock_wrlock(&(arraylist->rwlock)), strerror(errno), return errno);
    if (arraylist->item_count > item_count)
    {
        retval = EINVAL;
    }
    if (retval == 0)
    {
        retval = POOL_CPY_OUT(arraylist->pool, dst, arraylist->item_count - item_count, item_count, item_size, item_stride);
    }
    if (retval == 0)
    {
        arraylist->item_count += item_count;
    }
    CHECK_ERR(pthread_rwlock_unlock(&(arraylist->rwlock)), strerror(errno), return errno);
    CHECK_ERR(retval, strerror(errno), return errno);
    return retval;
}
int __arraylist_insert(arraylist_t *arraylist, void *src, size_t index, size_t item_count, size_t item_size, size_t item_stride)
{
    CHECK_ERR(
        arraylist == NULL
            ? EINVAL
            : EXIT_SUCCESS,
        strerror(errno), return errno);

    if (item_count == 0)
    {
        return 0;
    }

    int retval = 0;
    CHECK_ERR(pthread_rwlock_wrlock(&(arraylist->rwlock)), strerror(errno), return errno);
    if (arraylist->item_count + item_count > arraylist->max_count)
    {
        retval = EINVAL;
    }
    if (retval == 0)
    {
        retval = POOL_MEMMOVE(arraylist->pool, index, index + item_count, arraylist->item_count - index);
    }
    if (retval == 0)
    {
        retval = POOL_CPY_IN(arraylist->pool, src, index, item_count, item_size, item_stride);
    }
    if (retval == 0)
    {
        arraylist->item_count += item_count;
    }
    CHECK_ERR(pthread_rwlock_unlock(&(arraylist->rwlock)), strerror(errno), return errno);
    CHECK_ERR(retval, strerror(errno), return errno);
    return 0;
}
int __arraylist_remove(arraylist_t *arraylist, void *dst, size_t index, size_t item_count, size_t item_size, size_t item_stride)
{
    CHECK_ERR(
        arraylist == NULL
            ? EINVAL
            : EXIT_SUCCESS,
        strerror(errno), return errno);

    if (item_count == 0)
    {
        return 0;
    }

    int retval = 0;
    CHECK_ERR(pthread_rwlock_wrlock(&(arraylist->rwlock)), strerror(errno), return errno);
    if (index + item_count > arraylist->item_count)
    {
        retval = EINVAL;
    }
    if (retval == 0)
    {
        retval = POOL_CPY_OUT(arraylist->pool, dst, index, item_count, item_size, item_stride);
    }
    if (retval == 0)
    {
        retval = POOL_MEMMOVE(arraylist->pool, index + item_count, index, arraylist->item_count - (index + item_count));
    }
    if (retval == 0)
    {
        arraylist->item_count -= item_count;
    }
    CHECK_ERR(pthread_rwlock_unlock(&(arraylist->rwlock)), strerror(errno), return errno);
    CHECK_ERR(retval, strerror(errno), return errno);
    return 0;
}

#define DEFAULT_A_PUSH_BACK __arraylist_push_back
#define DEFAULT_A_POP_BACK __arraylist_pop_back
#define DEFAULT_A_INSERT __arraylist_insert
#define DEFAULT_A_REMOVE __arraylist_remove

int arraylist_init(
    arraylist_t *arraylist,
    size_t backing_buffer_length,
    void *backing_buffer,
    size_t item_size,
    size_t chunk_alignment,
    pool_cpy_in_func_t cpy_in,
    pool_cpy_out_func_t cpy_out,
    pool_memset_func_t memset,
    pool_memmove_func_t memmove,
    arraylist_push_back_func_t push_back,
    arraylist_pop_back_func_t pop_back,
    arraylist_insert_func_t insert,
    arraylist_remove_func_t remove)
{
    pool_init(&(arraylist->pool), backing_buffer_length, backing_buffer, item_size, chunk_alignment, cpy_in, cpy_out, memset, memmove);

    arraylist->item_count = 0;
    arraylist->max_count = arraylist->pool.buf_len / arraylist->pool.item_size;
    arraylist->push_back = (push_back == NULL) ? DEFAULT_A_PUSH_BACK : push_back;
    arraylist->pop_back = (pop_back == NULL) ? DEFAULT_A_POP_BACK : pop_back;
    arraylist->insert = (insert == NULL) ? DEFAULT_A_INSERT : insert;
    arraylist->remove = (remove == NULL) ? DEFAULT_A_REMOVE : remove;
    return 0;
}