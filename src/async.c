#include <string.h>
#include <assert.h>
#include "async.h"

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

int __queue_push(queue_t *q, size_t item_size, void *item)
{
    assert(q != NULL);
    assert(item_size <= q->item_size);
    assert(item != NULL);

    int retval = 0;
    pthread_mutex_lock(&(q->lock_queue));
    void *new_head = q->head + q->item_size;
    if (new_head + q->item_size > q->start + q->buf_len)
    {
        new_head = q->start;
    }

    if (q->mNumItems < q->mMaxItems && new_head != q->tail)
    {
        memcpy(q->head, item, item_size);
        q->head = new_head;
        q->mNumItems++;
        pthread_cond_broadcast(&(q->size_cond));
    }
    else
    {
        retval = -1;
    }
    pthread_mutex_unlock(&(q->lock_queue));
    return retval;
}

int __queue_pop(queue_t *q, size_t item_size, void *item)
{
    assert(q != NULL);
    assert(item_size <= q->item_size);
    assert(item != NULL);

    int retval = 0;
    pthread_mutex_lock(&(q->lock_queue));
    if (q->mNumItems > 0 && q->tail != q->head)
    {
        memcpy(item, q->tail, item_size);
        q->tail += q->item_size;
        if (q->tail + q->item_size > q->start + q->buf_len)
        {
            q->tail = q->start;
        }
        q->mNumItems--;
        pthread_cond_broadcast(&(q->size_cond));
    }
    else
    {
        retval = -1;
    }
    pthread_mutex_unlock(&(q->lock_queue));
    return retval;
}

#define DEFAULT_Q_PUSH __queue_push
#define DEFAULT_Q_POP __queue_pop

void queue_init(
    queue_t *q,
    size_t backing_buffer_length,
    void *backing_buffer,
    size_t item_size,
    size_t chunk_alignment,
    push_func_t push_func,
    pop_func_t pop_func)
{
    assert(q != NULL &&
           "Queue cannot be NULL");
    assert(backing_buffer != NULL &&
           "Backing buffer cannot be NULL");

    // Align backing buffer to the specified chunk alignment
    void *start = (void *)align_forward((unsigned long)backing_buffer, (unsigned long)chunk_alignment);

    backing_buffer_length -= start - backing_buffer;

    // Align chunk size up to the required chunk_alignment
    item_size = align_forward(item_size, chunk_alignment);

    // Assert that the parameters passed are valid
    assert(backing_buffer_length >= item_size &&
           "Backing buffer length is smaller than the chunk size");

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
}

void *__workerFunction(void *args)
{
    assert(args != NULL);
    task_queue_t *tq = args;
    async_task_t task = {0};
    while (tq->run)
    {
        task.func = NULL;
        task.args = NULL;
        if (tq->queue.pop(&(tq->queue), sizeof(task), &task) || task.func == NULL)
        {
            pthread_mutex_lock(&(tq->queue.lock_queue));
            while (tq->queue.mNumItems <= 0)
                pthread_cond_wait(&(tq->queue.size_cond), &(tq->queue.lock_queue));
            pthread_mutex_unlock(&(tq->queue.lock_queue));
        }
        else
        {
            task.func(task.args);
        }
    }
    return tq;
}

#define DEFAULT_WORK_FUNC __workerFunction

void task_queue_init(
    task_queue_t *tq,
    size_t queue_buffer_length,
    void *queue_buffer,
    push_func_t push_func,
    pop_func_t pop_func,

    size_t worker_buffer_length,
    void *worker_buffer,
    work_func_t work_func)
{
    queue_init(&(tq->queue), queue_buffer_length, queue_buffer, sizeof(async_task_t), 8UL, push_func, pop_func);

    tq->work = work_func == NULL ? DEFAULT_WORK_FUNC : work_func;

    tq->worker_buffer_lenth = worker_buffer_length;
    tq->worker_buffer = worker_buffer;

    tq->run = 1;
    pthread_t *ptr_pthread = (pthread_t *)align_forward((unsigned long)tq->worker_buffer, 8UL);
    while (ptr_pthread + sizeof(*ptr_pthread) <= tq->worker_buffer + tq->worker_buffer_lenth)
    {
        pthread_create(ptr_pthread++, NULL, tq->work, tq);
    }
}