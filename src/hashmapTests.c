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

#define MAX_WORD 65536ul

int hashmap_test()
{
    program_state_t now_state = INIT;
    CHECK(set_current_state(&now_state), return EXIT_FAILURE);

    freelist_t wordcount_freelist;
    memset(&wordcount_freelist, 0, sizeof(freelist_t));
    CHECK(freelist_alloc(&wordcount_freelist, MAX_WORD, sizeof(wordcounter_t)), return EXIT_FAILURE);

    hashmap_t hashmap;
    memset(&hashmap, 0, sizeof(hashmap_t));
    CHECK(alloc_hashmap(&hashmap, NULL, MAX_WORD, MAX_WORD), return EXIT_FAILURE);

    FILE *f_ptr;
    f_ptr = fopen("./resources/pg100.txt", "r");
    THROW_ERR(!f_ptr, strerror(errno), return EXIT_FAILURE);

    char x[256];
    memset(x, 0, 256 * sizeof(char));
    char y[256];
    memset(y, 0, 256 * sizeof(char));

    now_state = RUN;
    CHECK(set_current_state(&now_state), return EXIT_FAILURE);

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

            CHECK(hashmap_find((void **)&wordcount, &hashmap, (unsigned char *)y, key_size), return EXIT_FAILURE);

            if (wordcount == NULL)
            {
                unsigned long int index = -1;
                CHECK(freelist_aquire(&index, &wordcount_freelist), return EXIT_FAILURE);
                CHECK(pool_get_ptr((void **)&wordcount, &(wordcount_freelist.pool), index), return EXIT_FAILURE);

                wordcount->count = 1;
                memset(&(wordcount->word_buff), 0, MAX_KEY_LEN * sizeof(unsigned char));
                memcpy(&(wordcount->word_buff), y, key_size);
                CHECK(hashmap_add(wordcount, &hashmap, (unsigned char *)y, key_size), return EXIT_FAILURE);
            }
            else
            {
                wordcount->count += 1;
            }
        }

        memset(x, 0, 256 * sizeof(char));
        memset(y, 0, 256 * sizeof(char));
    }

    now_state = INIT;
    CHECK(set_current_state(&now_state), return EXIT_FAILURE);

    fclose(f_ptr);
    CHECK(hashmap_print_nodes(&hashmap), return EXIT_FAILURE);
    CHECK(free_hashmap(&hashmap), return EXIT_FAILURE);
    CHECK(freelist_free(&wordcount_freelist), return EXIT_FAILURE);

    return 0;
}