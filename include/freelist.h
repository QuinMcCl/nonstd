#ifndef FREELIST_H
#define FREELIST_H
#include "pool.h"

typedef struct freelist_node_s
{
    long int next;
} freelist_node_t;

typedef struct freelist_s
{
    unsigned long int first;
    pool_t pool;
} freelist_t;




int freelist_alloc(freelist_t *free_list, unsigned long int item_count, unsigned long int item_size);
int freelist_free(freelist_t *free_list);

int freelist_aquire(unsigned long int *dst, freelist_t *free_list);
int freelist_release(unsigned long int *src, freelist_t *free_list);

#endif