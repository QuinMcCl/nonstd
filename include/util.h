#ifndef UTIL_H
#define UTIL_H

#include <assert.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int is_power_of_two(unsigned long x);
    unsigned long align_forward(unsigned long ptr, unsigned long align);
    unsigned long align_backward(unsigned long ptr, unsigned long align);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif