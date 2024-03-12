#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define QUEUE_PUSH(Q, I, B) Q.push(&(Q), sizeof(I), (void *)&I, B)
#define QUEUE_POP(Q, I, B) Q.pop(&(Q), sizeof(I), (void *)&I, B)

    typedef struct queue_s queue_t;
    typedef int (*push_func_t)(queue_t *q, size_t item_size, const void *item, int blocking);
    typedef int (*pop_func_t)(queue_t *q, size_t item_size, void *item, int blocking);
    struct queue_s
    {
        unsigned char *buf;
        size_t buf_len;
        size_t item_size;

        pthread_mutex_t lock_queue;
        pthread_cond_t size_cond;
        unsigned long mNumItems;
        unsigned long mMaxItems;
        void *start;
        void *head;
        void *tail;

        push_func_t push;
        pop_func_t pop;
    };

    int queue_init(
        queue_t *q,
        size_t backing_buffer_length,
        void *backing_buffer,
        size_t item_size,
        size_t chunk_alignment,
        push_func_t push_func,
        pop_func_t pop_func);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* QUEUE_H */