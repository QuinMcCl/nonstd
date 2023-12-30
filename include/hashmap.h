#ifndef HASHMAP_H
#define HASHMAP_H

#include "pool.h"
#include "freelist.h"

#define MAX_KEY_LEN 256

typedef unsigned long int (*hashfunc_t)(unsigned char *key, unsigned long int key_size);


typedef struct hash_node_s
{
    unsigned long int key_size;
    unsigned char key_buff[MAX_KEY_LEN];
    unsigned long int next;
    void *data;
} hash_node_t;

typedef struct hashmap_s
{
    hashfunc_t hashfunc;
    pool_t hash_pool;
    freelist_t hashnode_freelist;
} hashmap_t;

int alloc_hashmap(hashmap_t *map, hashfunc_t hashfunc, unsigned long int hashpool_count, unsigned long int hashnode_count);
int free_hashmap(hashmap_t *map);
int hashmap_add(   void  *data, hashmap_t *map, unsigned char *key, unsigned long int key_size);
int hashmap_remove(void **data, hashmap_t *map, unsigned char *key, unsigned long int key_size);
int hashmap_find(  void **data, hashmap_t *map, unsigned char *key, unsigned long int key_size);
int hashmap_print_nodes(hashmap_t *map);



#endif