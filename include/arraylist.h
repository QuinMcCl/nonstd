#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include "pool.h"

typedef struct arraylist_s
{
    unsigned long int item_count;
    pool_t pool;
} arraylist_t;

int arraylist_alloc(arraylist_t *arraylist, unsigned long int item_count, unsigned long int item_size);
int arraylist_free(arraylist_t *arraylist);

int arraylist_push_back(void *src, arraylist_t *arraylist, unsigned long int item_count);
int arraylist_pop_back(void *dst, arraylist_t *arraylist, unsigned long int item_count);
int arraylist_insert(void *src, arraylist_t *arraylist, unsigned long int start, unsigned long int item_count);
int arraylist_remove(void *dst, arraylist_t *arraylist, unsigned long int start, unsigned long int item_count);

#endif