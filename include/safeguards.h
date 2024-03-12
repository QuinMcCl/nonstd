#ifndef SAFEGUARDS_H
#define SAFEGUARDS_H

#ifndef ERROR_CHECKING
#define ERROR_CHECKING
#endif

#ifdef ERROR_CHECKING
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
// #ifndef ON_ERROR
// #define ON_ERROR
// #endif
#define CHECK_ERR(x)                                                                      \
    do                                                                                    \
    {                                                                                     \
        errno = (x);                                                                      \
        if (errno != EXIT_SUCCESS)                                                        \
        {                                                                                 \
            fprintf(stderr, "%s at %s:%d %s\n", strerror(errno), __FILE__, __LINE__, #x); \
            fflush(stderr);                                                               \
            ON_ERROR                                                                      \
        }                                                                                 \
    } while (0)

#else

#define CHECK_ERR(x) x;

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
    int safe_free(void **ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif