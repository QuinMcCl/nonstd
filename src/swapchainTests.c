#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "safeguards.h"
#include "swapchain.h"

void *backthreadfunc(void *arg)
{
    struct timespec ts = {0l, 500000l};
    swapchain_t *chain = (swapchain_t *)arg;
    unsigned long int value[2] = {0ul, 100ul};
    swaplink_t *link = NULL;
    unsigned long int *buffer = NULL;

    program_state_t now_state = INVALID_STATE;
    CHECK_ERR(get_current_state(&now_state), strerror(errno), pthread_exit(NULL));

    while (now_state != STOP)
    {
        if (now_state == RUN)
        {
            CHECK_ERR(SWAPCHAIN_WRITE_BACK((*chain), link, buffer), strerror(errno), pthread_exit(NULL));
            if (buffer != NULL)
            {
                buffer[0] = value[0]++;
                if (value[1] > 0)
                    buffer[1] = value[1]--;
                else
                    buffer[1] = 0;
            }
            CHECK_ERR(SWAPCHAIN_WRITE_CLOSE((*chain), link), strerror(errno), pthread_exit(NULL));
            link = NULL;
            buffer = NULL;
        }
        CHECK_ERR(get_current_state(&now_state), strerror(errno), pthread_exit(NULL));
        ts.tv_sec = 0l;
        // ts.tv_nsec = 500000000l; // 2hz
        ts.tv_nsec = 1000000l; // 1000hz
        while (nanosleep(&ts, &ts))
            ;
    };

    pthread_exit(arg);
}

void *frontthreadfunc(void *arg)
{
    struct timespec ts = {1l, 0l};
    swapchain_t *chain = (swapchain_t *)arg;
    swaplink_t *link = NULL;
    unsigned long int *buffer = NULL;

    program_state_t now_state = INVALID_STATE;
    CHECK_ERR(get_current_state(&now_state), strerror(errno), pthread_exit(NULL));

    while (now_state != STOP)
    {
        if (now_state == RUN)
        {
            CHECK_ERR(SWAPCHAIN_READ_FRONT((*chain), link, buffer), strerror(errno), pthread_exit(NULL));
            if (buffer != NULL)
            {
                fprintf(stdout, "%lu:%lu\n", buffer[0], buffer[1]);
                fflush(stdout);
            }
            CHECK_ERR(SWAPCHAIN_READ_CLOSE((*chain), link), strerror(errno), pthread_exit(NULL));
            link = NULL;
            buffer = NULL;
        }
        CHECK_ERR(get_current_state(&now_state), strerror(errno), pthread_exit(NULL));
        ts.tv_sec = 0l;
        // ts.tv_nsec = 16666666l; // 60 hz
        ts.tv_nsec = 100000000l; // 10hz
        while (nanosleep(&ts, &ts))
            ;
    };

    pthread_exit(arg);
}

struct bufferitem
{
    swaplink_t link __attribute__((__aligned__(8)));
    unsigned long int value[2] __attribute__((__aligned__(8)));
};

int swapchain_tests()
{
    struct timespec ts;
    program_state_t now_state = INIT;
    struct bufferitem backingBuffer[2] __attribute__((__aligned__(8))) = {0};
    swapchain_t test_swapchain = {0};
    pthread_t backThread;
    pthread_t frontThread;

    CHECK_ERR(set_current_state(now_state), strerror(errno), return EXIT_FAILURE);

    CHECK_ERR(swapchain_init(&test_swapchain, sizeof(backingBuffer), backingBuffer, sizeof(backingBuffer[0].value), 8UL, NULL, NULL, NULL, NULL, NULL), strerror(errno), return EXIT_FAILURE);

    CHECK_ERR(pthread_create(&backThread, NULL, backthreadfunc, &test_swapchain), strerror(errno), return EXIT_FAILURE);

    CHECK_ERR(pthread_create(&frontThread, NULL, frontthreadfunc, &test_swapchain), strerror(errno), return EXIT_FAILURE);

    now_state = RUN;
    CHECK_ERR(set_current_state(now_state), strerror(errno), return EXIT_FAILURE);

    ts.tv_sec = 10l; // RUN FOR 10 SECONDS
    ts.tv_nsec = 0l;
    // for (unsigned int i = 0; i < 100; i++)
    // {
    //     backthreadfunc(&test_swapchain);
    //     frontthreadfunc(&test_swapchain);
    // }
    while (nanosleep(&ts, &ts))
        ;

    now_state = STOP;
    CHECK_ERR(set_current_state(now_state), strerror(errno), return EXIT_FAILURE);

    CHECK_ERR(pthread_join(backThread, NULL), strerror(errno), return EXIT_FAILURE);
    CHECK_ERR(pthread_join(frontThread, NULL), strerror(errno), return EXIT_FAILURE);

    return 0;
}