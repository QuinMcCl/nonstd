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

#define ON_ERROR return errno;
int arraylist_test()
{
    srand(0);
    arraylist_t list = {0};
    double backingBufffer[ARRAY_SIZE] __attribute__((__aligned__(8))) = {0};
    double a[MOVE_COUNT] __attribute__((__aligned__(8))) = {0};
    double src[ARRAY_SIZE] __attribute__((__aligned__(8))) = {0};

    arraylist_init(&list, sizeof(backingBufffer), (void *)backingBufffer, sizeof(double), 8UL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    for (unsigned int i = 0; i < ARRAY_SIZE; i++)
    {
        src[i] = (double)rand() / (double)RAND_MAX;
        fprintf(stdout, "%f\t", src[i]);
    }
    fprintf(stdout, "\n");

    CHECK_ERR(ARRAYLIST_PUSH_BACK(list, src, ARRAY_SIZE, sizeof(src[0]), sizeof(src[0])));

    for (unsigned long int i = 0; i < ITERMAX; i++)
    {
        CHECK_ERR(ARRAYLIST_REMOVE(list, a, (rand() % (ARRAY_SIZE - MOVE_COUNT)), MOVE_COUNT, sizeof(a[0]), sizeof(a[0])));
        CHECK_ERR(ARRAYLIST_INSERT(list, a, (rand() % (ARRAY_SIZE - MOVE_COUNT)), MOVE_COUNT, sizeof(a[0]), sizeof(a[0])));
    }

    CHECK_ERR(ARRAYLIST_POP_BACK(list, src, ARRAY_SIZE, sizeof(src[0]), sizeof(src[0])));

    for (unsigned int i = 0; i < ARRAY_SIZE; i++)
    {
        fprintf(stdout, "%f\t", src[i]);
    }
    fprintf(stdout, "\n");
    return 0;
}

#undef ON_ERROR