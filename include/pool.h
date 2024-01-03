#ifndef POOL_H
#define POOL_H

#include <pthread.h>

typedef struct pool_s
{
    unsigned long int block_size;
    unsigned long int stride;
    unsigned long int max_count;
    unsigned char *block;

    pthread_rwlock_t rwlock;
} pool_t;

int pool_alloc(pool_t *pool, unsigned long int max_count, unsigned long int stride);
int pool_free(pool_t *pool);
int pool_get_ptr(void **ptr, pool_t *pool, unsigned long int start);
int pool_cpy_out(void *dst, pool_t *pool, unsigned long int start, unsigned long int count);
int pool_cpy_in(void *src, pool_t *pool, unsigned long int start, unsigned long int count);
int pool_cpy_in_b(pool_t *pool, void *src, unsigned long int start, unsigned long int count);
int pool_cpy_out_b(pool_t *pool, void *dst, unsigned long int start, unsigned long int count);
int pool_memset_b(pool_t *pool, int val, unsigned long int start, unsigned long int count);
int pool_move(pool_t *pool, unsigned long int sourc, unsigned long int desti, unsigned long int count);

#endif