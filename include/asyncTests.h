#ifndef ASYNC_TESTS_H
#define ASYNC_TESTS_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct worker_test_args_s worker_test_args_t;
    struct worker_test_args_s
    {
        unsigned long index;
        pthread_mutex_t done_lock;
        pthread_cond_t done_cond;
        unsigned int done;

        struct timespec enqueue_time;
        struct timespec dequeue_time;

    };


    int async_test();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ASYNC_TESTS_H */
