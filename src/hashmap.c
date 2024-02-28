#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "safeguards.h"
#include "hashmap.h"

size_t MurmurOAAT32(size_t key_size, const unsigned char *key)
{
    // unsigned long int h = 3323198485ul;
    size_t h = 18706ul * 36577ul;
    for (size_t i = 0; i < key_size; i++)
    {
        h ^= key[i];
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

size_t JenkinsOAAT32(size_t key_size, const unsigned char *key)
{
    // unsigned long int h = 3323198485ul;
    size_t h = 24851ul * 20764ul;
    for (size_t i = 0; i < key_size; i++)
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

int __hashmap_get_hash_node(hashmap_t *map, size_t key_size, const unsigned char *key, hash_node_t ***hash_node_ptr, hash_node_t **hash_node)
{
    CHECK_ERR(map == NULL || key_size <= 0 || key == NULL || hash_node_ptr == NULL || hash_node == NULL ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);
    // size_t hash_index = map->hashfunc(key, key_size) & (map->hash_count - 1ul); // IFF HASHCOUNT IS PWER OF 2
    size_t hash_index = map->hash(key_size, key) % (map->hash_count);

    *hash_node_ptr = &(map->hash_node_ptr_array[hash_index]);

    while (**hash_node_ptr != NULL)
    {
        *hash_node = **hash_node_ptr;
        if ((*hash_node)->key_size == key_size && !memcmp(&((*hash_node)->key_buff), key, key_size))
        {
            break;
        }
        *hash_node_ptr = &((*hash_node)->next);
        *hash_node = NULL;
    };

    return 0;
}

int __hashmap_add(hashmap_t *map, size_t key_size, const unsigned char *key, void *data)
{
    CHECK_ERR(map == NULL || key_size <= 0 || key == NULL || data == NULL ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);

    hash_node_t **hash_node_ptr = NULL;
    hash_node_t *hash_node = NULL;

    int retval = 0;

    CHECK_ERR(pthread_rwlock_wrlock(&(map->rw_lock)), strerror(errno), return errno);

    retval = __hashmap_get_hash_node(map, key_size, key, &hash_node_ptr, &hash_node);

    if (!retval && hash_node_ptr != NULL && *hash_node_ptr == NULL && hash_node == NULL)
    {
        *hash_node_ptr = map->first_free_node;
        map->first_free_node = map->first_free_node->next;

        hash_node = *hash_node_ptr;

        hash_node->key_size = key_size;
        memset(&(hash_node->key_buff), 0, MAX_KEY_LEN * sizeof(unsigned char));
        memcpy(&(hash_node->key_buff), key, key_size);
        hash_node->next = NULL;
        hash_node->data = data;
    }
    CHECK_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return errno);
    CHECK_ERR(retval, strerror(errno), return errno);
    return 0;
}

int __hashmap_remove(hashmap_t *map, size_t key_size, const unsigned char *key, void **data)
{
    CHECK_ERR(map == NULL || key_size <= 0 || key == NULL || data == NULL ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);

    hash_node_t **hash_node_ptr = NULL;
    hash_node_t *hash_node = NULL;

    int retval = 0;

    CHECK_ERR(pthread_rwlock_wrlock(&(map->rw_lock)), strerror(errno), return errno);

    retval = __hashmap_get_hash_node(map, key_size, key, &hash_node_ptr, &hash_node);

    if (!retval && hash_node_ptr != NULL && *hash_node_ptr != NULL && hash_node != NULL)
    {
        *data = hash_node->data;
        hash_node->next = map->first_free_node;
        map->first_free_node = hash_node;
        *hash_node_ptr = NULL;
    }
    CHECK_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return errno);
    CHECK_ERR(retval, strerror(errno), return errno);
    return 0;
}

int __hashmap_find(hashmap_t *map, size_t key_size, const unsigned char *key, void **data)
{
    CHECK_ERR(map == NULL || key_size <= 0 || key == NULL || data == NULL ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);

    hash_node_t **hash_node_ptr = NULL;
    hash_node_t *hash_node = NULL;

    int retval = 0;
    CHECK_ERR(pthread_rwlock_rdlock(&(map->rw_lock)), strerror(errno), return errno);

    retval = __hashmap_get_hash_node(map, key_size, key, &hash_node_ptr, &hash_node);

    if (!retval && hash_node_ptr != NULL && *hash_node_ptr != NULL && hash_node != NULL)
    {
        *data = hash_node->data;
    }

    CHECK_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return errno);
    CHECK_ERR(retval, strerror(errno), return errno);
    return 0;
}

#define DEFAULT_HASHMAP_HASH MurmurOAAT32
#define DEFAULT_HASHMAP_ADD __hashmap_add
#define DEFAULT_HASHMAP_REMOVE __hashmap_remove
#define DEFAULT_HASHMAP_FIND __hashmap_find

int hashmap_init(
    hashmap_t *map,
    size_t hash_count,
    hash_node_t **hash_node_ptr_array,
    size_t hash_node_count,
    hash_node_t *hash_node_array,
    hashmap_hash_func_t hash_func,
    hashmap_add_func_t add_func,
    hashmap_remove_func_t remove_func,
    hashmap_find_func_t find_func)
{
    CHECK_ERR(map == NULL || hash_count <= 0 || hash_node_ptr_array == NULL || hash_node_count <= 0 || hash_node_array == NULL ? EINVAL : EXIT_SUCCESS, strerror(errno), return errno);

    map->hash_count = hash_count;
    map->hash_node_ptr_array = hash_node_ptr_array;
    map->hash_node_array = hash_node_array;
    map->first_free_node = &(map->hash_node_array[0]);
    for (size_t hash_node_index = 0; hash_node_index < (hash_node_count - 1); hash_node_index++)
    {
        map->hash_node_array[hash_node_index].next = &(map->hash_node_array[hash_node_index + 1]);
    }

    map->hash = hash_func == NULL ? (hashmap_hash_func_t)DEFAULT_HASHMAP_HASH : hash_func;
    map->add = add_func == NULL ? DEFAULT_HASHMAP_ADD : add_func;
    map->remove = remove_func == NULL ? DEFAULT_HASHMAP_REMOVE : remove_func;
    map->find = find_func == NULL ? DEFAULT_HASHMAP_FIND : find_func;

    return 0;
}

int hashmap_print_nodes(hashmap_t *map)
{

    unsigned long int depth = 0;
    hash_node_t *hash_node = NULL;

    CHECK_ERR(pthread_rwlock_rdlock(&(map->rw_lock)), strerror(errno), return errno);

    fprintf(stdout, "depth\tkey_size\tkey\tnext\tcount\n");

    for (unsigned int i = 0; i < map->hash_count; i++)
    {
        hash_node = map->hash_node_ptr_array[i];

        // if (hash_node == NULL)
        // {
        //     fprintf(stdout, "%ld\t%ld\t%s\t%ld\t%p\t%ld\n", -1l, -1l, "", -1l, NULL, -1l);
        // }

        depth = 0;
        while (hash_node != NULL)
        {
            const unsigned char *next_key;
            if (hash_node->next != NULL)
                next_key = hash_node->next->key_buff;
            else
                next_key = (const unsigned char *)"NULL";
            fprintf(stdout, "%ld\t%ld\t%s\t%s\t%ld\n", depth, hash_node->key_size, hash_node->key_buff, next_key, *(unsigned long int *)(hash_node->data));

            hash_node = hash_node->next;
            depth++;
        };
    }

    CHECK_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return errno);
    return 0;
}