#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "util.h"
#include "safeguards.h"
#include "queue.h"

#define ON_ERROR return errno;

int __queue_push(queue_t *q, size_t item_size, const void *item, int blocking)
{
    assert(q != NULL && "NULL QUEUE PTR");
    assert(item_size <= q->item_size && "ITEM TOO LARGE");
    assert(item != NULL && "NULL ITEM PTR");

    int retval = EXIT_SUCCESS;
    CHECK_ERR(pthread_mutex_lock(&(q->lock_queue)));

    while (blocking && q->mNumItems >= q->mMaxItems)
        retval = pthread_cond_wait(&(q->size_cond), &(q->lock_queue));

    void *new_head = q->head + q->item_size;
    if (new_head + q->item_size > q->start + q->buf_len)
    {
        new_head = q->start;
    }
    if (!retval && q->mNumItems < q->mMaxItems && new_head != q->tail)
    {
        memcpy(q->head, item, item_size);
        q->head = new_head;
        q->mNumItems++;
        retval = pthread_cond_broadcast(&(q->size_cond));
    }
    else
    {
        retval = EAGAIN;
    }

    CHECK_ERR(pthread_mutex_unlock(&(q->lock_queue)));
    CHECK_ERR(retval);
    return EXIT_SUCCESS;
}

int __queue_pop(queue_t *q, size_t item_size, void *item, int blocking)
{
    assert(q != NULL && "NULL QUEUE PTR");
    assert(item_size <= q->item_size && "ITEM TOO LARGE");
    assert(item != NULL && "NULL ITEM PTR");

    int retval = EXIT_SUCCESS;
    CHECK_ERR(pthread_mutex_lock(&(q->lock_queue)));

    while (blocking && q->mNumItems <= 0)
        retval = pthread_cond_wait(&(q->size_cond), &(q->lock_queue));

    if (!retval && q->mNumItems > 0 && q->tail != q->head)
    {
        memcpy(item, q->tail, item_size);
        q->tail += q->item_size;
        if (q->tail + q->item_size > q->start + q->buf_len)
        {
            q->tail = q->start;
        }
        q->mNumItems--;
        CHECK_ERR(pthread_cond_broadcast(&(q->size_cond)));
    }
    else
    {
        item = NULL;
        // retval = EAGAIN;
    }

    CHECK_ERR(pthread_mutex_unlock(&(q->lock_queue)));
    // if (retval && retval != EAGAIN)
    CHECK_ERR(retval);
    return EXIT_SUCCESS;
}

#define DEFAULT_Q_PUSH __queue_push
#define DEFAULT_Q_POP __queue_pop

int queue_init(
    queue_t *q,
    size_t backing_buffer_length,
    void *backing_buffer,
    size_t item_size,
    size_t chunk_alignment,
    push_func_t push_func,
    pop_func_t pop_func)
{
    assert(q != NULL && "NULL QUEUE PTR");
    assert(backing_buffer != NULL && "NULL BUFFER PTR");

    // Align backing buffer to the specified chunk alignment
    void *start = (void *)align_forward((unsigned long)backing_buffer, (unsigned long)chunk_alignment);

    backing_buffer_length -= start - backing_buffer;

    // Align chunk size up to the required chunk_alignment
    item_size = align_forward(item_size, chunk_alignment);

    assert(backing_buffer_length >= item_size && "BUFFER TOO SMALL");

    // Store the adjusted parameters
    q->buf = (unsigned char *)backing_buffer;
    q->buf_len = backing_buffer_length;
    q->item_size = item_size;

    q->mNumItems = 0ul;
    q->mMaxItems = (q->buf_len / q->item_size) - 1ul;
    q->start = start;
    q->head = start;
    q->tail = start;

    q->push = push_func == NULL ? DEFAULT_Q_PUSH : push_func;
    q->pop = pop_func == NULL ? DEFAULT_Q_POP : pop_func;

    return EXIT_SUCCESS;
}

#undef ON_ERROR