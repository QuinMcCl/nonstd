#ifndef POOL_H
#define POOL_H
/*

A pool should
1. Allocate a block of memory of a fixed size once
2. provide safe indexed access into the byte array
3. deallocate at the end

*/

typedef struct pool_s
{
    unsigned long int block_size;
    unsigned long int stride;
    unsigned long int max_count;
    unsigned char *block;
} pool_t;


int pool_alloc(pool_t *pool, unsigned long int max_count, unsigned long int stride);
int pool_free(pool_t *pool);
int pool_get_ptr(void **ptr, pool_t *pool, unsigned long int start);
int pool_cpy_out(void *dst,  pool_t *pool, unsigned long int start, unsigned long int count);
int pool_cpy_in (void *src,  pool_t *pool, unsigned long int start, unsigned long int count);
int pool_move(pool_t *pool, unsigned long int sourc, unsigned long int desti, unsigned long int count);

#endif