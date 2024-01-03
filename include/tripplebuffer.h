#ifndef TRIPPLEBUFFER_H
#define TRIPPLEBUFFER_H

#include <pthread.h>

typedef enum buffer_name_e
{
    FRONT,
    MIDDLE,
    BACK
} buffer_name_t;

typedef struct tripplebuffer_s
{
    unsigned long int block_size;
    unsigned long int stride;
    unsigned long int max_count;

    int swap_ready;

    unsigned char *front;
    unsigned char *middle;
    unsigned char *back;

    pthread_mutex_t swap_lock;

} tripplebuffer_t;

int tripplebuffer_alloc(tripplebuffer_t *tripplebuffer, unsigned long int max_count, unsigned long int stride);
int tripplebuffer_free(tripplebuffer_t *tripplebuffer);

int tripplebuffer_get_ptr(void **ptr, tripplebuffer_t *tripplebuffer, unsigned long int start, buffer_name_t buffer_name);

int tripplebuffer_cpy_out_front(void *dst, tripplebuffer_t *tripplebuffer, unsigned long int start, unsigned long int count);
int tripplebuffer_cpy_in_back(void *src, tripplebuffer_t *tripplebuffer, unsigned long int start, unsigned long int count);

int tripplebuffer_swap_front(tripplebuffer_t *tripplebuffer);
int tripplebuffer_swap_back(tripplebuffer_t *tripplebuffer);

#endif