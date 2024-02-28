#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <pthread.h>
#include "pool.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ARRAYLIST_PUSH_BACK(A, SRC, CNT, SRC_SIZE, SRC_STRIDE) A.push_back(&(A), SRC, CNT, SRC_SIZE, SRC_STRIDE)
#define ARRAYLIST_POP_BACK(A, DST, CNT, DST_SIZE, DST_STRIDE) A.pop_back(&(A), DST, CNT, DST_SIZE, DST_STRIDE)
#define ARRAYLIST_INSERT(A, SRC, IND, CNT, SRC_SIZE, SRC_STRIDE) A.insert(&(A), SRC, IND, CNT, SRC_SIZE, SRC_STRIDE)
#define ARRAYLIST_REMOVE(A, DST, IND, CNT, DST_SIZE, DST_STRIDE) A.remove(&(A), DST, IND, CNT, DST_SIZE, DST_STRIDE)

    typedef struct arraylist_s arraylist_t;
    typedef int (*arraylist_push_back_func_t)(arraylist_t *arraylist, void *src, size_t item_count, size_t item_size, size_t item_stride);
    typedef int (*arraylist_pop_back_func_t)(arraylist_t *arraylist, void *dst, size_t item_count, size_t item_size, size_t item_stride);
    typedef int (*arraylist_insert_func_t)(arraylist_t *arraylist, void *src, size_t index, size_t item_count, size_t item_size, size_t item_stride);
    typedef int (*arraylist_remove_func_t)(arraylist_t *arraylist, void *dst, size_t index, size_t item_count, size_t item_size, size_t item_stride);

    struct arraylist_s
    {
        pool_t pool;

        size_t item_count;
        size_t max_count;
        pthread_rwlock_t rwlock;

        arraylist_push_back_func_t push_back;
        arraylist_pop_back_func_t pop_back;
        arraylist_insert_func_t insert;
        arraylist_remove_func_t remove;
    };

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
        arraylist_remove_func_t remove);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif