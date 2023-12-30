#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "safeguards.h"
#include "arraylist.h"

int arraylist_alloc(arraylist_t *arraylist, unsigned long int item_count, unsigned long int item_size)
{
#ifdef ERROR_CHECKING
    if (arraylist == NULL)
        THROW_ERR(-1, "NULL ARRAYLIST PTR", return retval);
#endif
    CHECK(pool_alloc(&(arraylist->pool), item_count, item_size), return retval);
    arraylist->item_count = 0;

    return 0;
}

int arraylist_free(arraylist_t *arraylist)
{
#ifdef ERROR_CHECKING
    if (arraylist == NULL)
        THROW_ERR(-1, "NULL ARRAYLIST PTR", return retval);
#endif
    CHECK(pool_free(&(arraylist->pool)), return retval);
    arraylist->item_count = -1;
    return 0;
}

int arraylist_push_back(void *src, arraylist_t *arraylist, unsigned long int item_count)
{
#ifdef ERROR_CHECKING
    if (arraylist == NULL)
        THROW_ERR(-1, "NULL ARRAYLIST PTR", return retval);
    if(arraylist->item_count + item_count > arraylist->pool.max_count)
        THROW_ERR(-1, "ARRAYLIST WILL NOT FIT ITEMS", return retval);
#endif

    CHECK(pool_cpy_in(src, &(arraylist->pool), arraylist->item_count, item_count), return retval);
    arraylist->item_count += item_count;

    return 0;
}
int arraylist_pop_back(void *dst, arraylist_t *arraylist, unsigned long int item_count)
{
#ifdef ERROR_CHECKING
    if (arraylist == NULL)
        THROW_ERR(-1, "NULL ARRAYLIST PTR", return retval);
    if(arraylist->item_count < item_count)
        THROW_ERR(-1, "ARRAYLIST DOES NOT HAVE ENOUGH ITEMS", return retval);
#endif

    CHECK(pool_cpy_out(dst, &(arraylist->pool), arraylist->item_count - item_count, item_count), return retval);
    arraylist->item_count -= item_count;

    return 0;
}
int arraylist_insert(void *src, arraylist_t *arraylist, unsigned long int start, unsigned long int item_count)
{
#ifdef ERROR_CHECKING
    if (arraylist == NULL)
        THROW_ERR(-1, "NULL ARRAYLIST PTR", return retval);
    if(arraylist->item_count + item_count > arraylist->pool.max_count)
        THROW_ERR(-1, "ARRAYLIST WILL NOT FIT ITEMS", return retval);
#endif

    CHECK(pool_move(&(arraylist->pool), start, start + item_count, arraylist->item_count - start), return retval);
    CHECK(pool_cpy_in(src, &(arraylist->pool), start, item_count), return retval);
    arraylist->item_count += item_count;
    return 0;
}
int arraylist_remove(void *dst, arraylist_t *arraylist, unsigned long int start, unsigned long int item_count)
{
#ifdef ERROR_CHECKING
    if (arraylist == NULL)
        THROW_ERR(-1, "NULL ARRAYLIST PTR", return retval);
    if(arraylist->item_count < item_count)
        THROW_ERR(-1, "ARRAYLIST DOES NOT HAVE ENOUGH ITEMS", return retval);
#endif

    CHECK(pool_cpy_out(dst, &(arraylist->pool), start, item_count), return retval);
    CHECK(pool_move(&(arraylist->pool), start + item_count, start, arraylist->item_count - start - item_count), return retval);
    arraylist->item_count -= item_count;

    return 0;
}