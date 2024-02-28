#ifndef POOL_H
#define POOL_H

#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define POOL_CPY_IN(P, SRC, IND, CNT, SRC_SIZE, SRC_STRIDE) P.cpy_in(&(P), (void *)SRC, IND, CNT, SRC_SIZE, SRC_STRIDE)
#define POOL_CPY_OUT(P, DST, IND, CNT, DST_SIZE, DST_STRIDE) P.cpy_out(&(P), (void *)DST, IND, CNT, DST_SIZE, DST_STRIDE)
#define POOL_MEMSET(P, VAL, IND, CNT) P.memset(&(P), VAL, IND, CNT)
#define POOL_MEMMOVE(P, SRC, DST, CNT) P.memmove(&(P), SRC, DST, CNT)

    typedef struct pool_s pool_t;
    typedef int (*pool_cpy_in_func_t)(pool_t *pool, void *src, size_t start_index, size_t count, size_t stride, size_t size);
    typedef int (*pool_cpy_out_func_t)(pool_t *pool, void *dst, size_t start_index, size_t count, size_t stride, size_t size);
    typedef int (*pool_memset_func_t)(pool_t *pool, int val, size_t start_index, size_t count);
    typedef int (*pool_memmove_func_t)(pool_t *pool, size_t sourc, size_t desti, size_t count);

    struct pool_s
    {
        unsigned char *buf;
        size_t buf_len;
        size_t item_size;
        pthread_rwlock_t rwlock;

        pool_cpy_in_func_t cpy_in;
        pool_cpy_out_func_t cpy_out;
        pool_memset_func_t memset;
        pool_memmove_func_t memmove;
    };

    int pool_init(
        pool_t *pool,
        size_t backing_buffer_length,
        void *backing_buffer,
        size_t item_size,
        size_t chunk_alignment,
        pool_cpy_in_func_t cpy_in,
        pool_cpy_out_func_t cpy_out,
        pool_memset_func_t memset,
        pool_memmove_func_t memmove);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif