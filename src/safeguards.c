#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include "safeguards.h"
#define ON_ERROR return errno;

pthread_mutex_t state_lock;
pthread_cond_t state_cond;
program_state_t current_state = INIT;

int get_current_state(program_state_t *state)
{
    assert(state != NULL && "NULL State PTR");
    CHECK_ERR(pthread_mutex_lock(&(state_lock)));
    *state = current_state;
    CHECK_ERR(pthread_mutex_unlock(&(state_lock)));
    return 0;
}

int set_current_state(program_state_t state)
{
    assert(state < MAX_PROGRAM_STATE && "Invalid State Value");
    CHECK_ERR(pthread_mutex_lock(&(state_lock)));
    current_state = state;
    CHECK_ERR(pthread_cond_signal(&(state_cond)));
    CHECK_ERR(pthread_mutex_unlock(&(state_lock)));
    return 0;
}

int wait_until_state(program_state_t state)
{
    assert(state < MAX_PROGRAM_STATE && "Invalid State Value");
    CHECK_ERR(pthread_mutex_lock(&(state_lock)));
    while (current_state != state)
        CHECK_ERR(pthread_cond_wait(&(state_cond), &(state_lock)));
    CHECK_ERR(pthread_mutex_unlock(&(state_lock)));
    return 0;
}

int safe_alloc(void **ptr, unsigned long int size)
{
    assert(ptr != NULL && "NULL PTR VALUE");
    program_state_t now_state = INVALID_STATE;
    CHECK_ERR(get_current_state(&now_state));
    assert(now_state != RUN && "CANNOT ALLOCATE WHILE RUNNING");

    void *p = malloc(size);
    CHECK_ERR(errno);
    CHECK_ERR(!p && size ? ENOMEM : EXIT_SUCCESS);

#ifdef CLEAR_AFTER_ALLOC
    memset(p, 0, size);
    CHECK_ERR(errno);
#endif

    *ptr = p;
    return 0;
}

int safe_free(void **ptr)
{
    assert(ptr != NULL && "NULL PTR VALUE");
    program_state_t now_state = INVALID_STATE;
    CHECK_ERR(get_current_state(&now_state));
    assert(now_state != RUN && "CANNOT FREE WHILE RUNNING");

    free(*ptr);
    CHECK_ERR(errno);
    *ptr = NULL;
    return 0;
}

#undef ON_ERROR