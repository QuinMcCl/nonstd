#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "safeguards.h"
#include "queue.h"
#include "async.h"

void *__workerFunction(void *args)
{
    CHECK_ERR(args == NULL ? EINVAL : EXIT_SUCCESS, strerror(errno), return NULL);
    task_queue_t *tq = args;
    async_task_t task = {0};
    while (tq->run)
    {
        task.func = NULL;
        task.args = NULL;
        CHECK_ERR(QUEUE_POP(tq->queue, task, 1), strerror(errno), return NULL);

        if (task.func != NULL)
        {
            task.func(task.args);
        }
    }
    return args;
}

#define DEFAULT_WORK_FUNC __workerFunction

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
    CHECK_ERR(queue_init(&(tq->queue), queue_buffer_length, queue_buffer, sizeof(async_task_t), 8UL, push_func, pop_func), strerror(errno), return errno);

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