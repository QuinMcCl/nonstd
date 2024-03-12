#ifndef ASYNC_H
#define ASYNC_H

#include <pthread.h>
#include "queue.h"

#ifdef __cplusplus
extern "C"
{
#endif

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

        unsigned long num_worker;
        pthread_t *worker_array;
    };

    int task_queue_init(
        task_queue_t *tq,
        size_t queue_buffer_length,
        void *queue_buffer,
        push_func_t push_func,
        pop_func_t pop_func,
        unsigned long num_worker,
        pthread_t *worker_array,
        work_func_t work_func);
    int task_queue_close(task_queue_t *tq);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ASYNC_H */