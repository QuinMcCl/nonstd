
#ifndef FREELIST_STATIC_H
#define FREELIST_STATIC_H

#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define FREELIST_GET(FL, I) FL.get(&(FL), (void **)&I)
#define FREELIST_REL(FL, I) FL.rel(&(FL), (void *)I)

    typedef struct freelist_s freelist_t;
    typedef int (*freelist_get_func_t)(freelist_t *fl, void **item);
    typedef int (*freelist_rel_func_t)(freelist_t *fl, void *item);

    struct freelist_s
    {
        freelist_get_func_t get;
        freelist_rel_func_t rel;

        pthread_mutex_t lock_list;

        unsigned char *buf;
        void *first;
        void *start;
        size_t buf_len;
        size_t item_size;
    };

    int freelist_init(freelist_t *fl,
                      size_t backing_buffer_length,
                      void *backing_buffer,
                      size_t item_size,
                      size_t chunk_alignment,
                      freelist_get_func_t get_func,
                      freelist_rel_func_t rel_func);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif