#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "safeguards.h"
#include "queue.h"
#include "async.h"

#define ON_ERROR return NULL;
void *__workerFunction(void *args)
{
    assert(args != NULL);
    task_queue_t *tq = args;
    async_task_t task = {0};
    while (tq->run)
    {
        task.func = NULL;
        task.args = NULL;
        CHECK_ERR(QUEUE_POP(tq->queue, task, 1));

        if (task.func != NULL)
        {
            task.func(task.args);
        }
    }
    return args;
}

#define DEFAULT_WORK_FUNC __workerFunction

#undef ON_ERROR
#define ON_ERROR return errno;
int task_queue_init(
    task_queue_t *tq,
    size_t queue_buffer_length,
    void *queue_buffer,
    push_func_t push_func,
    pop_func_t pop_func,
    unsigned long num_worker,
    pthread_t *worker_array,
    work_func_t work_func)
{
    CHECK_ERR(queue_init(&(tq->queue), queue_buffer_length, queue_buffer, sizeof(async_task_t), 8UL, push_func, pop_func));

    tq->work = work_func == NULL ? DEFAULT_WORK_FUNC : work_func;

    tq->num_worker = num_worker;
    tq->worker_array = worker_array;

    tq->run = 1;

    for (unsigned long worker_index = 0; worker_index < tq->num_worker; worker_index++)
    {
        pthread_create(&(tq->worker_array[worker_index]), NULL, tq->work, tq);
    }
    return 0;
}

int task_queue_close(task_queue_t *tq)
{
    async_task_t task = {0};
    tq->run = 0;

    task.func = NULL;
    task.funcName = "STOP";
    task.args = NULL;

    for (unsigned long worker_index = 0; worker_index < tq->num_worker; worker_index++)
    {
        QUEUE_PUSH(tq->queue, task, 1);
    }

    for (unsigned long worker_index = 0; worker_index < tq->num_worker; worker_index++)
    {
        pthread_join((tq->worker_array[worker_index]), NULL);
    }

    tq->work = NULL;
    tq->num_worker = 0;
    tq->worker_array = 0;
    return 0;
}

#undef ON_ERROR