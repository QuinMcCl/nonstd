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

int hashmap_alloc(hashmap_t *map, hashfunc_t hashfunc, unsigned long int hash_count, unsigned long int hash_node_count)
{
#ifdef ERROR_CHECKING
    THROW_ERR((map == NULL), "NULL MAP PTR", return retval);
    THROW_ERR((hash_count == 0), "INVALID HASH COUNT", return retval);
    THROW_ERR((hash_node_count == 0), "INVALID HASH_NODE COUNT", return retval);
#endif

    THROW_ERR(pthread_rwlock_wrlock(&(map->rw_lock)), strerror(errno), return retval);

    if (hashfunc == NULL)
    {
        map->hashfunc = &MurmurOAAT32;
    }
    else
    {
        map->hashfunc = hashfunc;
    }

    map->hash_count = hash_count;

    CHECK(safe_alloc((void **)&(map->hash_to_index), map->hash_count * sizeof(unsigned long int)), {
        THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
        return retval;
    });
    memset(map->hash_to_index, -1, map->hash_count * sizeof(unsigned long int));

    map->first_free_node = 0;
    map->hash_node_count = hash_node_count;
    CHECK(safe_alloc((void **)&(map->hash_node_list), map->hash_node_count * sizeof(hash_node_t)), {
        THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
        return retval;
    });
    memset(map->hash_node_list, 0, map->hash_node_count * sizeof(hash_node_t));

    for (unsigned int i = 0; i < map->hash_node_count; i++)
    {
        map->hash_node_list[i].next = i + 1;
    }

    THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
    return 0;
}

int hashmap_free(hashmap_t *map)
{
#ifdef ERROR_CHECKING
    THROW_ERR((map == NULL), "NULL MAP PTR", return retval);
    THROW_ERR((map->hash_count == 0), "INVALID HASH COUNT", return retval);
    THROW_ERR((map->hash_node_count == 0), "INVALID HASH_NODE COUNT", return retval);
#endif

    THROW_ERR(pthread_rwlock_wrlock(&(map->rw_lock)), strerror(errno), return retval);

    map->hashfunc = NULL;

    CHECK(safe_free((void **)&(map->hash_to_index), map->hash_count * sizeof(unsigned long int)), {
        THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
        return retval;
    });
    map->hash_count = 0;

    CHECK(safe_free((void **)&(map->hash_node_list), map->hash_node_count * sizeof(hash_node_t)), {
        THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
        return retval;
    });
    map->hash_node_count = 0;
    map->first_free_node = (unsigned long int)-1l;

    THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
    return 0;
}

int hashmap_get_hash_node_index(hashmap_t *map, unsigned char *key, unsigned long int key_size, unsigned long int **hash_node_index, hash_node_t **hash_node)
{
#ifdef ERROR_CHECKING
    THROW_ERR((map == NULL), "NULL MAP PTR", return retval);
    THROW_ERR((map->hash_count == 0), "INVALID HASH COUNT", return retval);
    THROW_ERR((map->hash_node_count == 0), "INVALID HASH_NODE COUNT", return retval);
    THROW_ERR((map->hashfunc == NULL), "NULL HASHFUNC PTR", return retval);
    THROW_ERR((key == NULL), "NULL KEY PTR", return retval);
    THROW_ERR((key_size == 0), "INVALID KEY SIZE", return retval);
    THROW_ERR((hash_node_index == NULL), "NULL HASN_NODE_INDEX PTR", return retval);
    THROW_ERR((hash_node == NULL), "NULL HASN_NODE PTR", return retval);
#endif

    unsigned long int hash_index = map->hashfunc(key, key_size) & (map->hash_count - 1ul);
    // unsigned long int hash_index = map->hashfunc(key, key_size) % (map->hash_pool.item_count);

    *hash_node_index = &(map->hash_to_index[hash_index]);

    while (**hash_node_index < map->hash_node_count)
    {
        *hash_node = &(map->hash_node_list[**hash_node_index]);
        if ((*hash_node)->key_size == key_size && !memcmp(&((*hash_node)->key_buff), key, key_size))
        {
            break;
        }
        *hash_node_index = &((*hash_node)->next);
        *hash_node = NULL;
    };

#ifdef ERROR_CHECKING
    THROW_ERR((*hash_node_index == NULL), "NULL HASH NODE INDEX PTR", return retval);
#endif

    return 0;
}

int hashmap_add(void *data, hashmap_t *map, unsigned char *key, unsigned long int key_size)
{
#ifdef ERROR_CHECKING
    THROW_ERR((map->first_free_node > map->hash_node_count), "MAP IS FULL", return retval);
#endif

    unsigned long int *hash_node_index = NULL;
    hash_node_t *hash_node = NULL;

    THROW_ERR(pthread_rwlock_wrlock(&(map->rw_lock)), strerror(errno), return retval);

    CHECK(hashmap_get_hash_node_index(map, key, key_size, &hash_node_index, &hash_node), {
        THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
        return retval;
    });

#ifdef ERROR_CHECKING
    THROW_ERR((*hash_node_index < map->hash_node_count), "KEY ALREADY EXISTS", {
        THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
        return retval;
    });
#endif
    *hash_node_index = map->first_free_node;
    hash_node = &(map->hash_node_list[*hash_node_index]);
    map->first_free_node = hash_node->next;

    hash_node->key_size = key_size;
    memset(&(hash_node->key_buff), 0, MAX_KEY_LEN * sizeof(unsigned char));
    memcpy(&(hash_node->key_buff), key, key_size);
    hash_node->next = (unsigned long int)-1l;
    hash_node->data = data;

    THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
    return 0;
}

int hashmap_remove(void **data, hashmap_t *map, unsigned char *key, unsigned long int key_size)
{
#ifdef ERROR_CHECKING
    THROW_ERR((data == NULL), "NULL DATA PTR", return retval);
#endif

    unsigned long int *hash_node_index = NULL;
    hash_node_t *hash_node = NULL;

    THROW_ERR(pthread_rwlock_wrlock(&(map->rw_lock)), strerror(errno), return retval);

    CHECK(hashmap_get_hash_node_index(map, key, key_size, &hash_node_index, &hash_node), {
        THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
        return retval;
    });

    if (*hash_node_index < map->hash_node_count)
    {
#ifdef ERROR_CHECKING
        THROW_ERR((hash_node == NULL), "NULL hash_node PTR", {
            THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
            return retval;
        });
#endif
        *data = hash_node->data;
        hash_node->next = map->first_free_node;
        map->first_free_node = *hash_node_index;
        *hash_node_index = (unsigned long int)-1l;
    }

    THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);

    return 0;
}

int hashmap_find(void **data, hashmap_t *map, unsigned char *key, unsigned long int key_size)
{
#ifdef ERROR_CHECKING
    THROW_ERR((data == NULL), "NULL DATA PTR", return retval);
#endif

    unsigned long int *hash_node_index = NULL;
    hash_node_t *hash_node = NULL;

    THROW_ERR(pthread_rwlock_rdlock(&(map->rw_lock)), strerror(errno), return retval);

    CHECK(hashmap_get_hash_node_index(map, key, key_size, &hash_node_index, &hash_node), {
        THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
        return retval;
    });

    if (*hash_node_index < map->hash_node_count)
    {
#ifdef ERROR_CHECKING
        THROW_ERR((hash_node == NULL), "NULL hash_node PTR", {
            THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
            return retval;
        });
#endif
        *data = hash_node->data;
    }

    THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
    return 0;
}

int hashmap_print_nodes(hashmap_t *map)
{
#ifdef ERROR_CHECKING
    THROW_ERR((map == NULL), "NULL MAP PTR", return retval);
#endif

    unsigned long int *hash_node_index = NULL;
    unsigned long int depth = 0;
    hash_node_t *hash_node = NULL;


    THROW_ERR(pthread_rwlock_rdlock(&(map->rw_lock)), strerror(errno), return retval);

    fprintf(stdout, "depth\tkey_size\tkey\tnext\tdata_ptr\tcount\n");

    for (unsigned int i = 0; i < map->hash_count; i++)
    {
        hash_node_index = &(map->hash_to_index[i]);

        if (*hash_node_index > map->hash_node_count)
        {
            fprintf(stdout, "%ld\t%ld\t%s\t%ld\t%p\t%ld\n", -1l, -1l, "", -1l, NULL, -1l);
        }

        depth = 0;
        while (*hash_node_index < map->hash_node_count)
        {
            hash_node = &(map->hash_node_list[*hash_node_index]);
            fprintf(stdout, "%ld\t%ld\t%s\t%ld\t%p\t%ld\n", depth, hash_node->key_size, hash_node->key_buff, hash_node->next, hash_node->data, *(unsigned long int *)(hash_node->data));

            hash_node_index = &(hash_node->next);
            depth++;
        };
    }

    THROW_ERR(pthread_rwlock_unlock(&(map->rw_lock)), strerror(errno), return retval);
    return 0;
}