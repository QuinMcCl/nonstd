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

    THROW_ERR(pthread_rwlock_wrlock(&(pool->rwlock)), strerror(errno), return retval);
    pool->max_count = max_count;
    pool->stride = stride;
    pool->block_size = max_count * stride;
    CHECK(safe_alloc((void **)&(pool->block), pool->block_size), {
        THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
        return retval;
    });
    THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);

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

    THROW_ERR(pthread_rwlock_wrlock(&(pool->rwlock)), strerror(errno), return retval);
    CHECK(safe_free((void **)&(pool->block), pool->block_size), {
        THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
        return retval;
    });

    pool->max_count = 0;
    pool->stride = 0;
    pool->block_size = 0;
    THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);

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
    THROW_ERR(pthread_rwlock_rdlock(&(pool->rwlock)), strerror(errno), return retval);
    void *src = NULL;
    CHECK(pool_get_ptr(&src, pool, start), {
        THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
        return retval;
    });

#ifdef ERROR_CHECKING
    if (src == NULL)
        THROW_ERR(-1, "COULD NOT GET SRC PTR", {
            THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
            return retval;
        });
#endif

    memcpy(dst, src, (count * pool->stride));

    THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
    return 0;
}

int pool_cpy_in(void *src, pool_t *pool, unsigned long int start, unsigned long int count)
{
#ifdef ERROR_CHECKING
    if (src == NULL)
        THROW_ERR(-1, "CANNOT COPY FROM NULL PTR", return retval);
    if ((start + count) > pool->max_count)
        THROW_ERR(-1, "TRYING TO COPY OUTSIDE OF BUFFER", return retval);
#endif

    THROW_ERR(pthread_rwlock_wrlock(&(pool->rwlock)), strerror(errno), return retval);

    void *dst = NULL;
    CHECK(pool_get_ptr(&dst, pool, start), {
        THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
        return retval;
    });

#ifdef ERROR_CHECKING
    if (dst == NULL)
        THROW_ERR(-1, "COULD NOT GET DST PTR", {
            THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
            return retval;
        });
#endif

    memcpy(dst, src, (count * pool->stride));

    THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);

    return 0;
}

int pool_cpy_in_b(pool_t *pool, void *src, unsigned long int start, unsigned long int count)
{
#ifdef ERROR_CHECKING
    if (src == NULL)
        THROW_ERR(-1, "CANNOT COPY FROM NULL PTR", return retval);
    if (((start * pool->stride) + count) > pool->block_size)
        THROW_ERR(-1, "TRYING TO COPY OUTSIDE OF BUFFER", return retval);
#endif

    THROW_ERR(pthread_rwlock_wrlock(&(pool->rwlock)), strerror(errno), return retval);

    void *dst = NULL;
    CHECK(pool_get_ptr(&dst, pool, start), {
        THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
        return retval;
    });

#ifdef ERROR_CHECKING
    if (dst == NULL)
        THROW_ERR(-1, "COULD NOT GET DST PTR", {
            THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
            return retval;
        });
#endif

    memcpy(dst, src, count);

    THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
    return 0;
}

int pool_cpy_out_b(pool_t *pool, void *dst, unsigned long int start, unsigned long int count)
{
#ifdef ERROR_CHECKING
    if (dst == NULL)
        THROW_ERR(-1, "CANNOT WRITE TO NULL PTR", return retval);
    if (((start * pool->stride) + count) > pool->block_size)
        THROW_ERR(-1, "TRYING TO COPY OUTSIDE OF BUFFER", return retval);
#endif

    THROW_ERR(pthread_rwlock_rdlock(&(pool->rwlock)), strerror(errno), return retval);

    void *src = NULL;
    CHECK(pool_get_ptr(&src, pool, start), {
        THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
        return retval;
    });

#ifdef ERROR_CHECKING
    if (src == NULL)
        THROW_ERR(-1, "COULD NOT GET SRC PTR", {
            THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
            return retval;
        });
#endif

    memcpy(dst, src, count);

    THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
    return 0;
}

int pool_memset_b(pool_t *pool, int val, unsigned long int start, unsigned long int count)
{

#ifdef ERROR_CHECKING
    if (((start * pool->stride) + count) > pool->block_size)
        THROW_ERR(-1, "TRYING TO COPY OUTSIDE OF BUFFER", return retval);
#endif

    THROW_ERR(pthread_rwlock_rdlock(&(pool->rwlock)), strerror(errno), return retval);

    void *dst = NULL;
    CHECK(pool_get_ptr(&dst, pool, start), {
        THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
        return retval;
    });

#ifdef ERROR_CHECKING
    if (dst == NULL)
        THROW_ERR(-1, "COULD NOT GET DST PTR", {
            THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
            return retval;
        });
#endif

    memset(dst, val, count);

    THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
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

    THROW_ERR(pthread_rwlock_wrlock(&(pool->rwlock)), strerror(errno), return retval);

    void *src = NULL;
    CHECK(pool_get_ptr(&src, pool, sourc), {
        THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
        return retval;
    });
    void *dst = NULL;
    CHECK(pool_get_ptr(&dst, pool, desti), {
        THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
        return retval;
    });

#ifdef ERROR_CHECKING
    if (src == NULL)
        THROW_ERR(-1, "COULD NOT GET SRC PTR", {
            THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
            return retval;
        });
    if (dst == NULL)
        THROW_ERR(-1, "COULD NOT GET DST PTR", {
            THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);
            return retval;
        });
#endif

    memmove(dst, src, (count * pool->stride));

    THROW_ERR(pthread_rwlock_unlock(&(pool->rwlock)), strerror(errno), return retval);

    return 0;
}
