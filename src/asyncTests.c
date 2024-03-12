#include <assert.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>

#include "safeguards.h"
#include "queue.h"
#include "async.h"
#include "asyncTests.h"

#define ON_ERROR return errno;
unsigned long fibb(unsigned long i)
{
    if (i == 0ul || i == 1ul)
        return i;
    else
        return fibb(i - 1ul) + fibb(i - 2ul);
}

#undef ON_ERROR
#define ON_ERROR return;
void worker_test(void *args)
{
    assert(args != NULL);
    worker_test_args_t *test_args = args;

    fprintf(stdout, "worker starting %lu\n", test_args->index);
    unsigned long fib = fibb(test_args->index);

    fprintf(stdout, "worker done %lu, fib: %lu\n", test_args->index, fib);
    CHECK_ERR(pthread_mutex_lock(&(test_args->done_lock)));
    test_args->done = 1;
    CHECK_ERR(pthread_cond_broadcast(&(test_args->done_cond)));
    CHECK_ERR(pthread_mutex_unlock(&(test_args->done_lock)));
}

#undef ON_ERROR
#define ON_ERROR return errno;
int async_test()
{
#define MAX_CONCURRENT_TASKS 17UL
#define MAX_THREADS 16UL
#define NUM_TASKS 48

    static async_task_t task_buffer[MAX_CONCURRENT_TASKS] __attribute__((__aligned__(8))) = {0};
    static pthread_t threads[MAX_THREADS] __attribute__((__aligned__(8))) = {0};

    worker_test_args_t args[NUM_TASKS] = {0};

    task_queue_t tq = {0};

    CHECK_ERR(task_queue_init(&tq, sizeof(task_buffer), task_buffer, NULL, NULL, MAX_THREADS, threads, NULL));

    for (unsigned long task_index = 0; task_index < NUM_TASKS; task_index++)
    {

        args[task_index].index = task_index;
        args[task_index].done = 0ul;
        clock_gettime(CLOCK_MONOTONIC, &(args[task_index].enqueue_time));

        async_task_t task = {0};
        task.func = worker_test;
        task.args = &(args[task_index]);

        QUEUE_PUSH(tq.queue, task, 1);
        fprintf(stdout, "Started %lu\n", task_index);
    }

    for (unsigned long task_index = 0; task_index < NUM_TASKS; task_index++)
    {
        fprintf(stdout, "waiting on %lu\n", task_index);

        CHECK_ERR(pthread_mutex_lock(&(args[task_index].done_lock)));
        while (!args[task_index].done)
        {
            CHECK_ERR(pthread_cond_wait(&(args[task_index].done_cond), &(args[task_index].done_lock)));
        }
        CHECK_ERR(pthread_mutex_unlock(&(args[task_index].done_lock)));

        clock_gettime(CLOCK_MONOTONIC, &(args[task_index].dequeue_time));

        double diff_time = (double)(args[task_index].dequeue_time.tv_sec - args[task_index].enqueue_time.tv_sec) + (double)(args[task_index].dequeue_time.tv_nsec - args[task_index].enqueue_time.tv_nsec) / 1.0E9;
        fprintf(stdout, "TASK %lu WAITED %f SECONDS\n", task_index, diff_time);
    }

    return 0;
}

#undef ON_ERROR
#define ON_ERROR return;
void async_async_test(void *tq_ptr)
{
    assert(tq_ptr != NULL);
    worker_test_args_t args[NUM_TASKS] = {0};
    task_queue_t *tq = (task_queue_t *)tq_ptr;

    for (unsigned long task_index = 0; task_index < NUM_TASKS; task_index++)
    {

        args[task_index].index = task_index;
        args[task_index].done = 0ul;
        clock_gettime(CLOCK_MONOTONIC, &(args[task_index].enqueue_time));

        async_task_t task = {0};
        task.funcName = "worker_test";
        task.func = worker_test;
        task.args = &(args[task_index]);

        QUEUE_PUSH(tq->queue, task, 1);
        fprintf(stdout, "Started %lu\n", task_index);
    }

    for (unsigned long task_index = 0; task_index < NUM_TASKS; task_index++)
    {
        fprintf(stdout, "waiting on %lu\n", task_index);

        CHECK_ERR(pthread_mutex_lock(&(args[task_index].done_lock)));
        while (!args[task_index].done)
        {
            CHECK_ERR(pthread_cond_wait(&(args[task_index].done_cond), &(args[task_index].done_lock)));
        }
        CHECK_ERR(pthread_mutex_unlock(&(args[task_index].done_lock)));

        clock_gettime(CLOCK_MONOTONIC, &(args[task_index].dequeue_time));

        double diff_time = (double)(args[task_index].dequeue_time.tv_sec - args[task_index].enqueue_time.tv_sec) + (double)(args[task_index].dequeue_time.tv_nsec - args[task_index].enqueue_time.tv_nsec) / 1.0E9;
        fprintf(stdout, "TASK %lu WAITED %f SECONDS\n", task_index, diff_time);
    }
}
#undef ON_ERROR