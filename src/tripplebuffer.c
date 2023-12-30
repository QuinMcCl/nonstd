#include "string.h"
#include "safeguards.h"
#include "tripplebuffer.h"

int tripplebuffer_alloc(tripplebuffer_t *tripplebuffer, unsigned long int max_count, unsigned long int stride)
{
    tripplebuffer->stride = stride;
    tripplebuffer->max_count = max_count;
    tripplebuffer->block_size = max_count * stride;

    CHECK(safe_alloc((void **)&(tripplebuffer->front), tripplebuffer->block_size), return retval);

    CHECK(safe_alloc((void **)&(tripplebuffer->middle), tripplebuffer->block_size), return retval);

    CHECK(safe_alloc((void **)&(tripplebuffer->back), tripplebuffer->block_size), return retval);

    return 0;
}

int tripplebuffer_free(tripplebuffer_t *tripplebuffer)
{
    CHECK(safe_free((void **)&(tripplebuffer->front), tripplebuffer->block_size), return retval);

    CHECK(safe_free((void **)&(tripplebuffer->middle), tripplebuffer->block_size), return retval);

    CHECK(safe_free((void **)&(tripplebuffer->back), tripplebuffer->block_size), return retval);

    tripplebuffer->stride = 0;
    tripplebuffer->max_count = 0;
    tripplebuffer->block_size = 0;

    return 0;
}

int tripplebuffer_cpy_out_front(void *dst, tripplebuffer_t *tripplebuffer, unsigned long int start, unsigned long int count)
{
#ifdef ERROR_CHECKING
    if (tripplebuffer == NULL)
        THROW_ERR(-1, "NULL tripplebuffer pointer", return retval);
    if ((start + count) > tripplebuffer->max_count)
        THROW_ERR(-1, "Buffer overrun", return retval);
    if (tripplebuffer->block_size == 0)
        THROW_ERR(-1, "invalid block size", return retval);
    if (tripplebuffer->front == NULL)
        THROW_ERR(-1, "null front buffer", return retval);
    if (tripplebuffer->middle == NULL)
        THROW_ERR(-1, "null middle buffer", return retval);
#endif


    if (tripplebuffer->swap_ready)
    {
        THROW_ERR(pthread_mutex_lock(&(tripplebuffer->swap_lock)), strerror(errno), return retval);
        void *tmp = tripplebuffer->middle;
        tripplebuffer->middle = tripplebuffer->front;
        tripplebuffer->front = tmp;
        tripplebuffer->swap_ready = 0;
        THROW_ERR(pthread_mutex_unlock(&(tripplebuffer->swap_lock)), strerror(errno), return retval);
    }

    if (dst == NULL)
    {
        return 0;
    }
    
    void *src = (void *)&(tripplebuffer->front[start * tripplebuffer->stride]);

    memcpy(dst, src, (count * tripplebuffer->block_size));
    return 0;
}

int tripplebuffer_cpy_in_back(void *src, tripplebuffer_t *tripplebuffer, unsigned long int start, unsigned long int count)
{
#ifdef ERROR_CHECKING
    if (tripplebuffer == NULL)
        THROW_ERR(-1, "NULL tripplebuffer pointer", return retval);
    if ((start + count) > tripplebuffer->max_count)
        THROW_ERR(-1, "Buffer overrun", return retval);
    if (tripplebuffer->block_size == 0)
        THROW_ERR(-1, "invalid block size", return retval);
    if (tripplebuffer->back == NULL)
        THROW_ERR(-1, "null back buffer", return retval);
    if (tripplebuffer->middle == NULL)
        THROW_ERR(-1, "null middle buffer", return retval);
#endif

    void *dst = (void *)&(tripplebuffer->back[start * tripplebuffer->stride]);

    if (src == NULL)
    {
        memset(dst, 0, (count * tripplebuffer->stride));
    }
    else
    {
        memcpy(dst, src, (count * tripplebuffer->stride));
    }

    THROW_ERR(pthread_mutex_lock(&(tripplebuffer->swap_lock)), strerror(errno), return retval);
    void *tmp = tripplebuffer->middle;
    tripplebuffer->middle = tripplebuffer->back;
    tripplebuffer->back = tmp;
    tripplebuffer->swap_ready = 1;
    THROW_ERR(pthread_mutex_unlock(&(tripplebuffer->swap_lock)), strerror(errno), return retval);

    return 0;
}