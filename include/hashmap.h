#ifndef HASHMAP_H
#define HASHMAP_H

#include <pthread.h>

#include "pool.h"
#include "freelist.h"

#define MAX_KEY_LEN 256

#ifdef __cplusplus
extern "C"
{
#endif

#define HASHMAP_ADD(MAP, SIZE_KEY, KEY, DATA) MAP.add(&(MAP), SIZE_KEY, (const unsigned char *)KEY, (void *)DATA)
#define HASHMAP_REMOVE(MAP, SIZE_KEY, KEY, DATA) MAP.remove(&(MAP), SIZE_KEY, (const unsigned char *)KEY, (void **)&DATA)
#define HASHMAP_FIND(MAP, SIZE_KEY, KEY, DATA) MAP.find(&(MAP), SIZE_KEY, (const unsigned char *)KEY, (void **)&DATA)

    typedef struct hash_node_s hash_node_t;
    typedef struct hashmap_s hashmap_t;

    typedef size_t (*hashmap_hash_func_t)(size_t key_size, const unsigned char *key);
    typedef int (*hashmap_add_func_t)(hashmap_t *map, size_t key_size, const unsigned char *key, void *data);
    typedef int (*hashmap_remove_func_t)(hashmap_t *map, size_t key_size, const unsigned char *key, void **data);
    typedef int (*hashmap_find_func_t)(hashmap_t *map, size_t key_size, const unsigned char *key, void **data);

    struct hash_node_s
    {
        size_t key_size;
        unsigned char key_buff[MAX_KEY_LEN];
        hash_node_t *next;
        void *data;
    };

    struct hashmap_s
    {
        pthread_rwlock_t rw_lock;

        size_t hash_count;
        hash_node_t **hash_node_ptr_array;

        hash_node_t *hash_node_array;
        hash_node_t *first_free_node;

        hashmap_hash_func_t hash;
        hashmap_add_func_t add;
        hashmap_remove_func_t remove;
        hashmap_find_func_t find;
    };

    int hashmap_init(
        hashmap_t *map,
        size_t hash_count,
        hash_node_t **hash_node_ptr_array,
        size_t hash_node_count,
        hash_node_t *hash_node_array,
        hashmap_hash_func_t hash_func,
        hashmap_add_func_t add_func,
        hashmap_remove_func_t remove_func,
        hashmap_find_func_t find_func);
    int hashmap_print_nodes(hashmap_t *map);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif