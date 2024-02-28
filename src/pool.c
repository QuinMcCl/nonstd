#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "util.h"
#include "safeguards.h"
#include "pool.h"

void *__pool_get_ptr(pool_t *pool, size_t start_index)
{
    return (pool == NULL || start_index * pool->item_size > pool->buf_len) ? NULL : (void *)(((size_t)pool->buf) + (start_index * pool->item_size));
}

int __pool_cpy_in(pool_t *pool, void *src, size_t start_index, size_t count, size_t stride, size_t size)
{
    CHECK_ERR(
        pool == NULL || (start_index + count) * pool->item_size > pool->buf_len || size > pool->item_size
            ? EINVAL
            : EXIT_SUCCESS,
        strerror(errno), return errno);
    if (src == NULL || count <= 0 || size <= 0)
    {
        return 0;
    }

    CHECK_ERR(pthread_rwlock_rdlock(&(pool->rwlock)), strerror(errno), return errno);
    for (size_t index = 0; index < count; index++)
    {
        void *dst = __pool_get_ptr(pool, index + start_index);
        if (dst != NULL)
            memcpy(dst, (void *)(((size_t)src) + (start_index * stride)), size);
    }
    CHECK_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return errno);

    return 0;
}

int __pool_cpy_out(pool_t *pool, void *dst, size_t start_index, size_t count, size_t stride, size_t size)
{

    CHECK_ERR(
        pool == NULL || (start_index + count) * pool->item_size > pool->buf_len || size > pool->item_size
            ? EINVAL
            : EXIT_SUCCESS,
        strerror(errno), return errno);
    if (dst == NULL || count <= 0 || size <= 0)
    {
        return 0;
    }

    CHECK_ERR(pthread_rwlock_rdlock(&(pool->rwlock)), strerror(errno), return errno);
    for (size_t index = 0; index < count; index++)
    {
        void *src = __pool_get_ptr(pool, index + start_index);
        double *dst_double = (double *)(((size_t)dst) + (index * stride));
        if (src != NULL)
            memcpy((void *)dst_double, src, size);
    }
    CHECK_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return errno);

    return 0;
}

int __pool_memset(pool_t *pool, int val, size_t start_index, size_t count)
{
    CHECK_ERR(
        pool == NULL || (start_index + count) * pool->item_size > pool->buf_len
            ? EINVAL
            : EXIT_SUCCESS,
        strerror(errno), return errno);
    if (count <= 0)
    {
        return 0;
    }

    CHECK_ERR(pthread_rwlock_rdlock(&(pool->rwlock)), strerror(errno), return errno);
    void *dst = __pool_get_ptr(pool, start_index);
    if (dst != NULL)
        memset(dst, val, count * pool->item_size);
    CHECK_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return errno);

    return 0;
}

int __pool_memmove(pool_t *pool, size_t sourc, size_t desti, size_t count)
{
    CHECK_ERR(pool == NULL ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);
    CHECK_ERR((sourc + count) * pool->item_size > pool->buf_len ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);
    CHECK_ERR((desti + count) * pool->item_size > pool->buf_len ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);

    if (count <= 0 || sourc == desti)
    {
        return 0;
    }

    CHECK_ERR(pthread_rwlock_rdlock(&(pool->rwlock)), strerror(errno), return errno);
    void *src = __pool_get_ptr(pool, sourc);
    void *dst = __pool_get_ptr(pool, desti);
    if (src != NULL && dst != NULL)
        memmove(dst, src, (count * pool->item_size));
    CHECK_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return errno);

    return 0;
}

#define DEFAULT_P_CPY_IN __pool_cpy_in
#define DEFAULT_P_CPY_OUT __pool_cpy_out
#define DEFAULT_P_MEMSET __pool_memset
#define DEFAULT_P_MEMMOVE __pool_memmove

int pool_init(
    pool_t *pool,
    size_t backing_buffer_length,
    void *backing_buffer,
    size_t item_size,
    size_t chunk_alignment,
    pool_cpy_in_func_t cpy_in,
    pool_cpy_out_func_t cpy_out,
    pool_memset_func_t memset,
    pool_memmove_func_t memmove)
{
    CHECK_ERR(
        backing_buffer == NULL
            ? EINVAL
            : EXIT_SUCCESS,
        strerror(errno), return errno);

    // Align backing buffer to the specified chunk alignment
    void *start = (void *)align_forward((unsigned long)backing_buffer, (unsigned long)chunk_alignment);

    backing_buffer_length -= start - backing_buffer;

    // Align chunk size up to the required chunk_alignment
    item_size = align_forward(item_size, chunk_alignment);

    CHECK_ERR(
        backing_buffer_length < item_size
            ? EINVAL
            : EXIT_SUCCESS,
        strerror(errno), return errno);

    *pool = (pool_t){
        .buf = (unsigned char *)start,
        .buf_len = backing_buffer_length,
        .item_size = item_size,
        .cpy_in = (cpy_in == NULL) ? DEFAULT_P_CPY_IN : cpy_in,
        .cpy_out = (cpy_out == NULL) ? DEFAULT_P_CPY_OUT : cpy_out,
        .memset = (memset == NULL) ? DEFAULT_P_MEMSET : memset,
        .memmove = (memmove == NULL) ? DEFAULT_P_MEMMOVE : memmove,
    };
    return 0;
}
