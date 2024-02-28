#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "safeguards.h"
#include "pool.h"
#include "freelist.h"
#include "hashmap.h"

typedef struct wordcounter_s
{
    unsigned long int count;
    unsigned char word_buff[MAX_KEY_LEN];
} wordcounter_t;

#define MAX_WORD 65536UL

int hashmap_test()
{
    wordcounter_t *freelist_buffer = malloc(MAX_WORD * sizeof(wordcounter_t));
    hash_node_t **hash_node_ptr_array = malloc(MAX_WORD * sizeof(hash_node_t *));
    hash_node_t *hash_node_array = malloc(MAX_WORD * sizeof(hash_node_t));

    // wordcounter_t freelist_buffer[MAX_WORD] = {0};
    // hash_node_t *hash_node_ptr_array[MAX_WORD] = {0};
    // hash_node_t hash_node_array[MAX_WORD] = {0};

    freelist_t wordcount_freelist = {0};
    hashmap_t hashmap = {0};

    FILE *f_ptr;
    char x[256] = {0};
    char y[256] = {0};

    program_state_t now_state;

    now_state = INIT;
    CHECK_ERR(set_current_state(now_state), strerror(errno), return errno);

    freelist_init(&wordcount_freelist,
                  MAX_WORD * sizeof(wordcounter_t),
                  (void *)freelist_buffer,
                  sizeof(freelist_buffer[0]),
                  8UL,
                  NULL,
                  NULL);

    CHECK_ERR(hashmap_init(
                  &hashmap,
                  MAX_WORD,
                  hash_node_ptr_array,
                  MAX_WORD,
                  hash_node_array,
                  NULL,
                  NULL,
                  NULL,
                  NULL),
              strerror(errno), return errno);

    f_ptr = fopen("./resources/pg100.txt", "r");
    CHECK_ERR(!f_ptr, strerror(errno), return errno);

    now_state = RUN;
    CHECK_ERR(set_current_state(now_state), strerror(errno), return errno);

    while (fscanf(f_ptr, " %255s", x) == 1)
    {
        char *p = x;
        char *q = y;
        unsigned long int key_size = 0;

        for (; *p; p++)
            if (isalpha(*p))
            {
                *q = tolower(*p);
                q++;
                key_size++;
            }

        if (key_size > 0)
        {
            wordcounter_t *wordcount = NULL;

            // hashmap_find((void **)&wordcount, &hashmap, (unsigned char *)y, key_size);
            HASHMAP_FIND(hashmap, key_size, y, wordcount);

            if (wordcount == NULL)
            {
                FREELIST_GET(wordcount_freelist, wordcount);
                memset(wordcount, 0, sizeof(*wordcount));

                wordcount->count = 1;
                memcpy(&(wordcount->word_buff), y, key_size);
                HASHMAP_ADD(hashmap, key_size, (unsigned char *)y, wordcount);
            }
            else
            {
                wordcount->count += 1;
            }
        }

        memset(x, 0, 256 * sizeof(char));
        memset(y, 0, 256 * sizeof(char));
    }

    hashmap_print_nodes(&hashmap);
    now_state = STOP;
    set_current_state(now_state);
    fclose(f_ptr);

    return 0;
}