#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "safeguards.h"
#include "pool.h"

int pool_alloc(pool_t *pool, unsigned long int max_count, unsigned long int stride)
{
#ifdef ERROR_CHECKING
    if (pool == NULL)
        THROW_ERR(-1, "NULL POOL PTR", return retval);
    if (max_count == 0)
        THROW_ERR(-1, "INVALID MAX_COUNT", return retval);
    if (stride == 0)
        THROW_ERR(-1, "INVALID STRIDE", return retval);
#endif

    pool->max_count = max_count;
    pool->stride = stride;
    pool->block_size = max_count * stride;

    CHECK(safe_alloc((void **)&(pool->block), pool->block_size), return retval);

    return 0;
}

int pool_free(pool_t *pool)
{
#ifdef ERROR_CHECKING
    if (pool == NULL)
        THROW_ERR(-1, "NULL POOL PTR", return retval);
    if (pool->block == NULL)
        THROW_ERR(-1, "NULL POOL BLOCK PTR", return retval);
#endif

    CHECK(safe_free((void **)&(pool->block), pool->block_size), return retval);

    pool->max_count = 0;
    pool->stride = 0;
    pool->block_size = 0;

    return 0;
}

int pool_get_ptr(void **ptr, pool_t *pool, unsigned long int start)
{
#ifdef ERROR_CHECKING
    if (ptr == NULL)
        THROW_ERR(-1, "NULL PTR", return retval);
    if (*ptr != NULL)
        THROW_ERR(-1, "NON EMPTY PTR", return retval);
    if (pool == NULL)
        THROW_ERR(-1, "NULL POOL PTR", return retval);
    if (pool->block == NULL)
        THROW_ERR(-1, "NULL POOL BLOCK PTR", return retval);
    if (pool->block_size == 0)
        THROW_ERR(-1, "INVALID POOL BLOCK SIZE", return retval);
    if (pool->max_count == 0)
        THROW_ERR(-1, "INVALID POOL MAXCOUNT", return retval);
    if (pool->stride == 0)
        THROW_ERR(-1, "INVALID POOL STRIDE", return retval);
    if (start >= pool->max_count)
        THROW_ERR(-1, "INVALID START INDEX", return retval);
#endif
    *ptr = &(pool->block[start * pool->stride]);
    return 0;
}

int pool_cpy_out(void *dst, pool_t *pool, unsigned long int start, unsigned long int count)
{
    if (dst == NULL)
    {
        return 0;
    }

#ifdef ERROR_CHECKING
    if ((start + count) > pool->max_count)
        THROW_ERR(-1, "TRYING TO COPY OUTSIDE OF BUFFER", return retval);
#endif

    void *src = NULL;
    CHECK(pool_get_ptr(&src, pool, start), return retval);

#ifdef ERROR_CHECKING
    if (src == NULL)
        THROW_ERR(-1, "COULD NOT GET SRC PTR", return retval);
#endif

    memcpy(dst, src, (count * pool->stride));
    return 0;
}

int pool_cpy_in(void *src, pool_t *pool, unsigned long int start, unsigned long int count)
{
#ifdef ERROR_CHECKING
    if ((start + count) > pool->max_count)
        THROW_ERR(-1, "TRYING TO COPY OUTSIDE OF BUFFER", return retval);
#endif

    void *dst = NULL;
    CHECK(pool_get_ptr(&dst, pool, start), return retval);

#ifdef ERROR_CHECKING
    if (dst == NULL)
        THROW_ERR(-1, "COULD NOT GET DST PTR", return retval);
#endif

    if (src == NULL)
    {
        memset(dst, 0, (count * pool->stride));
    }
    else
    {
        memcpy(dst, src, (count * pool->stride));
    }

    return 0;
}

int pool_move(pool_t *pool, unsigned long int sourc, unsigned long int desti, unsigned long int count)
{
    if (count == 0 || sourc == desti)
    {
        return 0;
    }
#ifdef ERROR_CHECKING
    if ((sourc + count) > pool->max_count)
        THROW_ERR(-1, "TRYING TO MOVE OUTSIDE OF BUFFER", return retval);
    if ((desti + count) > pool->max_count)
        THROW_ERR(-1, "TRYING TO MOVE OUTSIDE OF BUFFER", return retval);
#endif
    void *src = NULL;
    CHECK(pool_get_ptr(&src, pool, sourc), return retval);
    void *dst = NULL;
    CHECK(pool_get_ptr(&dst, pool, desti), return retval);


#ifdef ERROR_CHECKING
    if (src == NULL)
        THROW_ERR(-1, "COULD NOT GET SRC PTR", return retval);
    if (dst == NULL)
        THROW_ERR(-1, "COULD NOT GET DST PTR", return retval);
#endif

    memmove(dst, src, (count * pool->stride));

    return 0;
}
