#ifndef SAFEGUARDS_H
#define SAFEGUARDS_H

#ifdef ERROR_CHECKING
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

#define CHECK_ERR(x, y, z)                                                  \
    do                                                                      \
    {                                                                       \
        errno = (x);                                                        \
        if (errno != EXIT_SUCCESS)                                          \
        {                                                                   \
            fprintf(stderr, "%s at %s:%d %s\n", y, __FILE__, __LINE__, #x); \
            fflush(stderr);                                                 \
            z;                                                              \
        }                                                                   \
    } while (0)

#else

#define CHECK_ERR(x, y, z) x;

#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum program_state_e
    {
        INVALID_STATE,
        INIT,
        MENU,
        RUN,
        STOP,
        MAX_PROGRAM_STATE,

    } program_state_t;

    int get_current_state(program_state_t *state);
    int set_current_state(program_state_t state);
    int wait_until_state(program_state_t state);

    int safe_alloc(void **ptr, unsigned long int size);
    int safe_free(void **ptr, unsigned long int size);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif