#ifndef TRIPPLEBUFFER_H
#define TRIPPLEBUFFER_H

#include <pthread.h> 

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

}tripplebuffer_t;




int tripplebuffer_alloc(tripplebuffer_t *tripplebuffer, unsigned long int max_count, unsigned long int stride);
int tripplebuffer_free(tripplebuffer_t *tripplebuffer);

int tripplebuffer_cpy_out_front(void *dst,  tripplebuffer_t *tripplebuffer, unsigned long int start, unsigned long int count);
int tripplebuffer_cpy_in_back (void *src,  tripplebuffer_t *tripplebuffer, unsigned long int start, unsigned long int count);

#endif