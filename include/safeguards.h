#ifndef SAFEGUARDS_H
#define SAFEGUARDS_H

#ifdef ERROR_CHECKING
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#define CHECK(x, z)                                                     \
    do                                                                  \
    {                                                                   \
        int retval = (x);                                               \
        if (retval != EXIT_SUCCESS)                                     \
        {                                                               \
            fprintf(stderr, "\tat %s:%s:%d\n", __FILE__, #x, __LINE__); \
            fflush(stderr);                                             \
            z;                                                          \
        }                                                               \
    } while (0)

#define THROW_ERR(x, y, z)                                                         \
    do                                                                             \
    {                                                                              \
        int retval = (x);                                                          \
        if (retval != EXIT_SUCCESS)                                                \
        {                                                                          \
            fprintf(stderr, "error at %s:%s:%d, %s\n", __FILE__, #x, __LINE__, y); \
            fflush(stderr);                                                        \
            z;                                                                     \
        }                                                                          \
    } while (0)

#else

#define CHECK(x, z) \
    do              \
    {               \
        (x);        \
    } while (0)

#define THROW_ERR(x, y, z) \
    do                     \
    {                      \
        (x);               \
    } while (0)

#endif

typedef enum program_state_e
{
    INVALID_STATE,
    INIT,
    MENU,
    RUN,
    STOP,
    
} program_state_t;

int get_current_state(program_state_t *state);
int set_current_state(const program_state_t state);
int wait_until_state(const program_state_t state);

int safe_alloc(void **ptr, unsigned long int size);
int safe_free(void **ptr, unsigned long int size);

#endif