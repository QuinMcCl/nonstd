#include "util.h"

int is_power_of_two(unsigned long x)
{
    return (x & (x - 1)) == 0;
}
unsigned long align_forward(unsigned long ptr, unsigned long align)
{
    unsigned long modulo;
    assert(is_power_of_two(align));
    modulo = ptr & (align - 1);

    if (modulo != 0)
    {
        ptr += align - modulo;
    }
    return ptr;
}
unsigned long align_backward(unsigned long ptr, unsigned long align)
{
    unsigned long modulo;
    assert(is_power_of_two(align));
    modulo = ptr & (align - 1);

    if (modulo != 0)
    {
        ptr -= modulo;
    }
    return ptr;
}