#include <assert.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include "async.h"
#include "asyncTests.h"

unsigned long fibb(unsigned long i)
{
    if (i == 0ul || i == 1ul)
    return i;
    else
        return fibb(i - 1ul) + fibb(i - 2ul);
}

void worker_test(void *args)
{
    assert(args != NULL);
    worker_test_args_t *test_args = args;

    fprintf(stdout, "worker starting %lu\n", test_args->index);
    unsigned long fib = fibb(test_args->index);


    fprintf(stdout, "worker done %lu, fib: %lu\n", test_args->index, fib);
    pthread_mutex_lock(&(test_args->done_lock));
    test_args->done = 1;
    pthread_cond_signal(&(test_args->done_cond));
    pthread_mutex_unlock(&(test_args->done_lock));
}

int async_test()
{
#define MAX_CONCURRENT_TASKS 1024UL
#define MAX_THREADS 16UL
#define NUM_TASKS 64

    async_task_t task_buffer[MAX_CONCURRENT_TASKS] = {0};
    pthread_t threads[MAX_THREADS] = {0};

    worker_test_args_t args[NUM_TASKS] = {0};

    task_queue_t tq = {0};

    task_queue_init(&tq, sizeof(task_buffer), task_buffer, NULL, NULL, sizeof(threads), threads, NULL, NULL, NULL);

    for (unsigned long task_index = 0; task_index < NUM_TASKS; task_index++)
    {
        fprintf(stdout, "Starting %lu\n", task_index);

        args[task_index].index = task_index;
        args[task_index].done = 0ul;
        clock_gettime(CLOCK_MONOTONIC, &(args[task_index].enqueue_time));

        async_task_t task = {0};
        task.func = worker_test;
        task.args = &(args[task_index]);

        while (tq.queue.push(&(tq.queue), sizeof(task), &task))
        {
            fprintf(stdout, "task queue full\n");
        }
        tq.awake(&tq);
    }

    for (unsigned long task_index = 0; task_index < NUM_TASKS; task_index++)
    {
        fprintf(stdout, "waiting on %lu\n", task_index);
        pthread_mutex_lock(&(args[task_index].done_lock));
        while (!args[task_index].done)
        {
            pthread_cond_wait(&(args[task_index].done_cond), &(args[task_index].done_lock));
        }
        pthread_mutex_unlock(&(args[task_index].done_lock));

        clock_gettime(CLOCK_MONOTONIC, &(args[task_index].dequeue_time));

        double diff_time = (double)(args[task_index].dequeue_time.tv_sec - args[task_index].enqueue_time.tv_sec) + (double)(args[task_index].dequeue_time.tv_nsec - args[task_index].enqueue_time.tv_nsec) / 1.0E9;
        fprintf(stdout, "TASK %lu WAITED %f SECONDS\n", task_index, diff_time);
    }

    return 0;
}