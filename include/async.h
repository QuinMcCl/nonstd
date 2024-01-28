#ifndef ASYNC_H
#define ASYNC_H

#include <stdatomic.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct queue_s queue_t;
    typedef int (*push_func_t)(queue_t *q, size_t item_size, void *item);
    typedef int (*pop_func_t)(queue_t *q, size_t item_size, void *item);
    struct queue_s
    {
        push_func_t push;
        pop_func_t pop;

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
    };

    typedef struct async_task_s async_task_t;
    struct async_task_s
    {
        void (*func)(void *args);
        const char *funcName;
        void *args;
    };

    typedef struct task_queue_s task_queue_t;
    typedef void *(*work_func_t)(void *);

    struct task_queue_s
    {
        int run;
        queue_t queue;

        work_func_t work;

        size_t worker_buffer_lenth;
        pthread_t *worker_buffer;
    };

    void task_queue_init(
        task_queue_t *tq,
        size_t queue_buffer_length,
        void *queue_buffer,
        push_func_t push_func,
        pop_func_t pop_func,

        size_t worker_buffer_length,
        void *worker_buffer,
        work_func_t work_func);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ASYNC_H */