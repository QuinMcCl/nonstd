#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "util.h"
#include "safeguards.h"
#include "freelist.h"

#define ON_ERROR return errno;
int __freelist_clr(freelist_t *fl)
{
    assert(fl != NULL && "NULL FREELIST PTR");
    CHECK_ERR(pthread_mutex_lock(&(fl->lock_list)));

    void **item = fl->start;

    while ((size_t)item + fl->item_size < (size_t)fl->start + fl->buf_len)
    {
        *item = (void *)(((size_t)item) + fl->item_size);
        item = *item;
    }
    CHECK_ERR(pthread_mutex_unlock(&(fl->lock_list)));
    return 0;
}

int __freelist_get(freelist_t *fl, void **item)
{
    assert(fl != NULL && "NULL FREELIST PTR");
    assert(item != NULL && "NULL ITEM PTR");

    CHECK_ERR(pthread_mutex_lock(&(fl->lock_list)));

    if (fl->first != NULL)
    {
        void **free = fl->first;
        fl->first = *free;
        *item = free;
    }

    CHECK_ERR(pthread_mutex_unlock(&(fl->lock_list)));
    return 0;
}

int __freelist_rel(freelist_t *fl, void *item)
{
    assert(fl != NULL && "NULL FREELIST PTR");
    assert(item >= fl->start && item < (void *)((size_t)fl->start + fl->buf_len) && "ITEM NOT IN FREELIST");

    CHECK_ERR(pthread_mutex_lock(&(fl->lock_list)));
    void **free = item;
    *free = fl->first;
    fl->first = item;
    CHECK_ERR(pthread_mutex_unlock(&(fl->lock_list)));
    return 0;
}

#define DEFAULT_FREELIST_GET __freelist_get
#define DEFAULT_FREELIST_REL __freelist_rel

int freelist_init(freelist_t *fl,
                  size_t backing_buffer_length,
                  void *backing_buffer,
                  size_t item_size,
                  size_t chunk_alignment,
                  freelist_get_func_t get_func,
                  freelist_rel_func_t rel_func)
{
    assert(fl != NULL && "NULL FREELIST PTR");
    assert(backing_buffer != NULL && "NULL BUFFER PTR");

    // Align backing buffer to the specified chunk alignment
    void *start = (void *)align_forward((unsigned long)backing_buffer, chunk_alignment);

    backing_buffer_length -= start - backing_buffer;

    // Align chunk size up to the required chunk_alignment
    item_size = align_forward(item_size, chunk_alignment);

    // Assert that the parameters passed are valid
    assert(backing_buffer_length >= item_size && "BUFFER TOO SMALL");

    // Store the adjusted parameters
    fl->buf = (unsigned char *)backing_buffer;
    fl->buf_len = backing_buffer_length;
    fl->item_size = item_size;

    fl->start = start;
    fl->first = start;

    CHECK_ERR(__freelist_clr(fl));

    fl->get = get_func == NULL ? DEFAULT_FREELIST_GET : get_func;
    fl->rel = rel_func == NULL ? DEFAULT_FREELIST_REL : rel_func;

    return 0;
}

#undef ON_ERROR