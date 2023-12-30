#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "safeguards.h"
#include "hashmap.h"


hashfunc_t default_hash_func;

unsigned long int MurmurOAAT32(unsigned char *key, unsigned long int key_size)
{
    // unsigned long int h = 3323198485ul;
    unsigned long int h = 18706ul * 36577ul;
    for (unsigned long i = 0; i < key_size; i++)
    {
        h ^= key[i];
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

unsigned long int JenkinsOAAT32(unsigned char *key, unsigned long int key_size)
{
    // unsigned long int h = 3323198485ul;
    unsigned long int h = 24851ul * 20764ul;
    for (unsigned long i = 0; i < key_size; i++)
    {
        h += key[i];
        h += h << 10;
        h ^= h >> 6;
    }

    h += h << 3;
    h ^= h >> 11;
    h += h << 15;
    return h;
}

int hashmap_get_hash_node_index(hashmap_t *map, unsigned char *key, unsigned long int key_size, unsigned long int **hash_node_index, hash_node_t **hash_node);

int alloc_hashmap(hashmap_t *map, hashfunc_t hashfunc, unsigned long int hashpool_count, unsigned long int hashnode_count)
{
    if (hashfunc == NULL)
    {
        map->hashfunc = &MurmurOAAT32;
        // map->hashfunc = &JenkinsOAAT32;
    }
    else
    {
        map->hashfunc = hashfunc;
    }

    CHECK(pool_alloc(&(map->hash_pool), hashpool_count, sizeof(unsigned long int)), return retval);

    memset(map->hash_pool.block, -1, map->hash_pool.block_size);

    CHECK(freelist_alloc(&(map->hashnode_freelist), hashnode_count, sizeof(hash_node_t)), return retval);
    return 0;
}

int free_hashmap(hashmap_t *map)
{

    CHECK(pool_free(&(map->hash_pool)), return retval);
    CHECK(freelist_free(&(map->hashnode_freelist)), return retval);
    return 0;
}

int hashmap_get_hash_node_index(hashmap_t *map, unsigned char *key, unsigned long int key_size, unsigned long int **hash_node_index, hash_node_t **hash_node)
{
#ifdef ERROR_CHECKING
    if (map == NULL)
        THROW_ERR(-1, "NULL MAP PTR", return retval);
    if (key == NULL)
        THROW_ERR(-1, "NULL KEY PTR", return retval);
    if (key_size == 0)
        THROW_ERR(-1, "INVALID KEY SIZE", return retval);
    if (map->hashfunc == NULL)
        THROW_ERR(-1, "NULL HASHFUNC PTR", return retval);
#endif

    unsigned long int hash_index = map->hashfunc(key, key_size) & (map->hash_pool.max_count - 1);
    // unsigned long int hash_index = map->hashfunc(key, key_size) % (map->hash_pool.item_count);

    CHECK(pool_get_ptr((void **)hash_node_index, &(map->hash_pool), hash_index), return retval);

    while (**hash_node_index < map->hashnode_freelist.pool.max_count)
    {
        *hash_node = (hash_node_t *)NULL;
        CHECK(pool_get_ptr((void **)hash_node, &(map->hashnode_freelist.pool), **hash_node_index), return retval);
        if ((*hash_node)->key_size == key_size && !memcmp(&((*hash_node)->key_buff), key, key_size))
        {
            break;
        }
        *hash_node_index = &((*hash_node)->next);
    };

#ifdef ERROR_CHECKING
    if (*hash_node_index == NULL)
        THROW_ERR(-1, "NULL HASH NODE INDEX PTR", return retval);
#endif

    return 0;
}

int hashmap_add(void *data, hashmap_t *map, unsigned char *key, unsigned long int key_size)
{
#ifdef ERROR_CHECKING
    if (data == NULL)
        THROW_ERR(-1, "NULL DATA PTR", return retval);
#endif

    unsigned long int *hash_node_index = NULL;
    hash_node_t *hash_node = NULL;
    CHECK(hashmap_get_hash_node_index(map, key, key_size, &hash_node_index, &hash_node), return retval);

#ifdef ERROR_CHECKING
    if (*hash_node_index < map->hashnode_freelist.pool.max_count)
        THROW_ERR(-1, "KEY ALREADY EXISTS", return retval);
#endif

    CHECK(freelist_aquire(hash_node_index, &(map->hashnode_freelist)), return retval);

    hash_node = (hash_node_t *)NULL;
    CHECK(pool_get_ptr((void **)&hash_node, &(map->hashnode_freelist.pool), *hash_node_index), return retval);

    hash_node->key_size = key_size;
    memset(&(hash_node->key_buff), 0, MAX_KEY_LEN * sizeof(unsigned char));
    memcpy(&(hash_node->key_buff), key, key_size);
    hash_node->next = (unsigned long int)(-1);
    hash_node->data = data;

    return 0;
}

int hashmap_remove(void **data, hashmap_t *map, unsigned char *key, unsigned long int key_size)
{
#ifdef ERROR_CHECKING
    if (data == NULL)
        THROW_ERR(-1, "NULL DATA PTR", return retval);
#endif

    unsigned long int *hash_node_index = NULL;
    hash_node_t *hash_node = NULL;
    CHECK(hashmap_get_hash_node_index(map, key, key_size, &hash_node_index, &hash_node), return retval);

    if (*hash_node_index < map->hashnode_freelist.pool.max_count)
    {
#ifdef ERROR_CHECKING
        if (hash_node == NULL)
            THROW_ERR(-1, "NULL hash_node PTR", return retval);
#endif
        *data = hash_node->data;

        CHECK(freelist_release(hash_node_index, &(map->hashnode_freelist)), return retval);
    }

    return 0;
}

int hashmap_find(void **data, hashmap_t *map, unsigned char *key, unsigned long int key_size)
{
#ifdef ERROR_CHECKING
    if (data == NULL)
        THROW_ERR(-1, "NULL DATA PTR", return retval);
#endif

    unsigned long int *hash_node_index = NULL;
    hash_node_t *hash_node = NULL;
    CHECK(hashmap_get_hash_node_index(map, key, key_size, &hash_node_index, &hash_node), return retval);

    if (*hash_node_index < map->hashnode_freelist.pool.max_count)
    {
#ifdef ERROR_CHECKING
        if (hash_node == NULL)
            THROW_ERR(-1, "NULL hash_node PTR", return retval);
#endif
        *data = hash_node->data;
    }

    return 0;
}

int hashmap_print_nodes(hashmap_t *map)
{
    unsigned long int *hash_node_index = NULL;
    unsigned long int depth = 0;
    hash_node_t *hash_node;
    fprintf(stdout, "depth\tkey_size\tkey\tnext\tdata_ptr\tcount\n");

    for (unsigned int i = 0; i < map->hash_pool.max_count; i++)
    {
        hash_node_index = (unsigned long int *)NULL;
        CHECK(pool_get_ptr((void **)&hash_node_index, &(map->hash_pool), i), return retval);

        if (*hash_node_index >= map->hashnode_freelist.pool.max_count)
        {
            fprintf(stdout, "%ld\t%ld\t%s\t%ld\t%p\t%ld\n", -1l, -1l, "", -1l, NULL, -1l);
        }

        depth = 0;
        while (*hash_node_index < map->hashnode_freelist.pool.max_count)
        {
            hash_node = (hash_node_t *)NULL;
            CHECK(pool_get_ptr((void **)&hash_node, &(map->hashnode_freelist.pool), *hash_node_index), return retval);

            fprintf(stdout, "%ld\t%ld\t%s\t%ld\t%p\t%ld\n", depth, hash_node->key_size, hash_node->key_buff, hash_node->next, hash_node->data, *(unsigned long int *)(hash_node->data));

            hash_node_index = &(hash_node->next);
            depth++;
        };
    }
    return 0;
}