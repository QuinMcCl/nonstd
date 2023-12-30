#include "string.h"
#include "pthread.h"
#include <time.h>

#include "safeguards.h"
#include "tripplebuffer.h"

void *backthreadfunc(void *arg)
{
    struct timespec ts = {0l, 500000l};
    tripplebuffer_t *tripplebuffer = (tripplebuffer_t *)arg;
    unsigned long int value[2] = {0ul, 100ul};

    program_state_t now_state = INVALID_STATE;
    CHECK(get_current_state(&now_state), pthread_exit(NULL));

    while (now_state != STOP)
    {
        if (now_state == RUN)
        {
            value[0]++;
            if (value[1])
                value[1]--;
            CHECK(tripplebuffer_cpy_in_back(value, tripplebuffer, 0, 2), pthread_exit(NULL));
        }
        CHECK(get_current_state(&now_state), pthread_exit(NULL));
        ts.tv_sec = 0l;
        ts.tv_nsec = 100000000l; // 10hz
        while (nanosleep(&ts, &ts))
            ;
    };

    pthread_exit(arg);
}

void *frontthreadfunc(void *arg)
{
    struct timespec ts = {1l, 0l};
    tripplebuffer_t *tripplebuffer = (tripplebuffer_t *)arg;
    unsigned long int value[2];

    program_state_t now_state = INVALID_STATE;
    CHECK(get_current_state(&now_state), pthread_exit(NULL));

    while (now_state != STOP)
    {
        if (now_state == RUN)
        {
            CHECK(tripplebuffer_cpy_out_front(value, tripplebuffer, 0, 2), pthread_exit(NULL));
            fprintf(stdout, "%lu:%lu\n", value[0], value[1]);
            fflush(stdout);
        }
        CHECK(get_current_state(&now_state), pthread_exit(NULL));
        ts.tv_sec = 0l;
        ts.tv_nsec = 16666666l; // 60 hz
        while (nanosleep(&ts, &ts))
            ;
    };

    pthread_exit(arg);
}

int tripplebuffer_tests()
{
    struct timespec ts;
    program_state_t now_state = INIT;
    CHECK(set_current_state(&now_state), return EXIT_FAILURE);

    tripplebuffer_t test_tripplebuffer;
    memset(&test_tripplebuffer, 0, sizeof(tripplebuffer_t));

    CHECK(tripplebuffer_alloc(&test_tripplebuffer, 2, sizeof(unsigned long int)), return EXIT_FAILURE);

    pthread_t backThread;
    THROW_ERR(pthread_create(&backThread, NULL, backthreadfunc, &test_tripplebuffer), strerror(errno), return EXIT_FAILURE);

    pthread_t frontThread;
    THROW_ERR(pthread_create(&frontThread, NULL, frontthreadfunc, &test_tripplebuffer), strerror(errno), return EXIT_FAILURE);

    now_state = RUN;
    CHECK(set_current_state(&now_state), return EXIT_FAILURE);

    ts.tv_sec = 10l; // RUN FOR 10 SECONDS
    ts.tv_nsec = 0l;
    while (nanosleep(&ts, &ts))
        ;

    now_state = STOP;
    CHECK(set_current_state(&now_state), return EXIT_FAILURE);

    THROW_ERR(pthread_join(backThread, NULL), strerror(errno), return EXIT_FAILURE);
    THROW_ERR(pthread_join(frontThread, NULL), strerror(errno), return EXIT_FAILURE);

    CHECK(tripplebuffer_free(&test_tripplebuffer), return EXIT_FAILURE);

    return 0;
}