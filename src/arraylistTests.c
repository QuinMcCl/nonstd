#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "safeguards.h"
#include "arraylist.h"
#include "arraylistTests.h"

#define ARRAY_SIZE 65536ul
#define ITERMAX 65536ul
#define MOVE_COUNT ARRAY_SIZE / 4ul

int arraylist_test()
{
    srand(0);
    arraylist_t list;
    memset(&list, 0, sizeof(list));
    CHECK(arraylist_alloc(&list, ARRAY_SIZE, sizeof(double)), return EXIT_FAILURE);

    double src[ARRAY_SIZE];
    for (unsigned int i = 0; i < ARRAY_SIZE; i++)
    {
        src[i] = (double)rand() / (double)RAND_MAX;
        fprintf(stdout, "%f\t", src[i]);
    }
    fprintf(stdout, "\n");

    CHECK(arraylist_push_back(&src, &list, ARRAY_SIZE), return EXIT_FAILURE);

    double a[MOVE_COUNT];
    for (unsigned int i = 0; i < ITERMAX; i++)
    {

        unsigned long int ai = rand() % (ARRAY_SIZE - MOVE_COUNT);
        unsigned long int bi = rand() % (ARRAY_SIZE - MOVE_COUNT);

        CHECK(arraylist_remove((void *)&a, &list, ai, MOVE_COUNT), return EXIT_FAILURE);
        CHECK(arraylist_insert((void *)&a, &list, bi, MOVE_COUNT), return EXIT_FAILURE);
    }

    CHECK(arraylist_pop_back(&src, &list, ARRAY_SIZE), return EXIT_FAILURE);

    for (unsigned int i = 0; i < ARRAY_SIZE; i++)
    {
        fprintf(stdout, "%f\t", src[i]);
    }
    fprintf(stdout, "\n");

    CHECK(arraylist_free(&list), return EXIT_FAILURE);

    return 0;
}