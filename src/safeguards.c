#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "safeguards.h"

pthread_rwlock_t state_lock;
program_state_t current_state = INIT;

int get_current_state(program_state_t *state)
{

    THROW_ERR(pthread_rwlock_rdlock(&(state_lock)), strerror(errno), return retval);
    *state = current_state;
    THROW_ERR(pthread_rwlock_unlock(&(state_lock)), strerror(errno), return retval);

    return 0;
}

int set_current_state(program_state_t *state)
{
    THROW_ERR(pthread_rwlock_wrlock(&(state_lock)), strerror(errno), return retval);
    current_state = *state;
    THROW_ERR(pthread_rwlock_unlock(&(state_lock)), strerror(errno), return retval);

    return 0;
}

int safe_alloc(void **ptr, unsigned long int size)
{

#ifdef ERROR_CHECKING
    if (ptr == NULL)
        THROW_ERR(-1, "NULL PTR", return retval);
    if (*ptr != NULL)
        THROW_ERR(-1, "NON-EMPTY PTR", return retval);
    if (size == 0)
        THROW_ERR(-1, "INVALID SIZE", return retval);
#endif

    program_state_t now_state = INVALID_STATE;
    CHECK(get_current_state(&now_state), return retval);

    if (now_state == RUN)
    {
        THROW_ERR(-1, "CANNOT ALLOCATE DURRING RUN", return retval);
    }

    void *p = malloc(size);
    if (!p)
    {
        THROW_ERR(-1, "COULD NOT ALLOCATE MEMORY", return retval);
    }

#ifdef CLEAR_AFTER_ALLOC
    memset(p, 0, size);
#endif

    *ptr = p;
    return 0;
}

int safe_free(void **ptr, unsigned long int size)
{
#ifdef ERROR_CHECKING
    if (ptr == NULL)
        THROW_ERR(-1, "NULL PTR", return retval);
    if (*ptr == NULL)
        THROW_ERR(-1, "NULL PTR", return retval);
    if (size == 0)
        THROW_ERR(-1, "INVALID SIZE", return retval);
#endif

    program_state_t now_state = INVALID_STATE;
    CHECK(get_current_state(&now_state), return retval);
    
    if (now_state == RUN)
    {
        THROW_ERR(-1, "CANNOT FREE DURRING RUN", return retval);
    }

#ifdef CLEAR_BEFORE_FREE
    memset(*ptr, 0, size);
#endif
    free(*ptr);
    *ptr = NULL;

    return 0;
}