#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>

#include "safeguards.h"

pthread_mutex_t state_lock;
pthread_cond_t state_cond;
program_state_t current_state = INIT;

int get_current_state(program_state_t *state)
{
    CHECK_ERR(state == NULL ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);
    CHECK_ERR(pthread_mutex_lock(&(state_lock)), strerror(errno), return errno);
    *state = current_state;
    CHECK_ERR(pthread_mutex_unlock(&(state_lock)), strerror(errno), return errno);
    return 0;
}

int set_current_state(program_state_t state)
{
    CHECK_ERR(state >= MAX_PROGRAM_STATE ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);
    CHECK_ERR(pthread_mutex_lock(&(state_lock)), strerror(errno), return errno);
    current_state = state;
    CHECK_ERR(pthread_cond_signal(&(state_cond)), strerror(errno), return errno);
    CHECK_ERR(pthread_mutex_unlock(&(state_lock)), strerror(errno), return errno);
    return 0;
}

int wait_until_state(program_state_t state)
{
    CHECK_ERR(state >= MAX_PROGRAM_STATE ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);
    CHECK_ERR(pthread_mutex_lock(&(state_lock)), strerror(errno), return errno);
    while (current_state != state)
        CHECK_ERR(pthread_cond_wait(&(state_cond), &(state_lock)), strerror(errno), return errno);
    CHECK_ERR(pthread_mutex_unlock(&(state_lock)), strerror(errno), return errno);
    return 0;
}

int safe_alloc(void **ptr, unsigned long int size)
{
    CHECK_ERR(ptr == NULL ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);
    program_state_t now_state = INVALID_STATE;
    CHECK_ERR(get_current_state(&now_state), strerror(errno), return errno);
    CHECK_ERR(now_state == RUN ? EPERM : EXIT_SUCCESS, strerror(errno), return errno);

    void *p = malloc(size);
    CHECK_ERR(errno, strerror(errno), return errno);
    CHECK_ERR(!p && size ? ENOMEM : EXIT_SUCCESS, strerror(errno), return errno);

#ifdef CLEAR_AFTER_ALLOC
    memset(p, 0, size);
    CHECK_ERR(errno, strerror(errno), return errno);
#endif

    *ptr = p;
    return 0;
}

int safe_free(void **ptr, unsigned long int size)
{
    CHECK_ERR(ptr == NULL || size == 0 ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);
    program_state_t now_state = INVALID_STATE;
    CHECK_ERR(get_current_state(&now_state), strerror(errno), return errno);
    CHECK_ERR(now_state == RUN ? EPERM : EXIT_SUCCESS, strerror(errno), return errno);

#ifdef CLEAR_BEFORE_FREE
    memset(*ptr, 0, size);
    CHECK_ERR(errno, strerror(errno), return errno);
#endif

    free(*ptr);
    CHECK_ERR(errno, strerror(errno), return errno);
    *ptr = NULL;
    return 0;
}