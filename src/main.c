#include "safeguards.h"
#include "arraylist.h"
#include "hashmap.h"
#include "tripplebuffer.h"
#include "arraylistTests.h"
#include "hashmapTests.h"
#include "tripplebufferTests.h"

int main()
{
    CHECK(arraylist_test(), return EXIT_FAILURE);
    CHECK(hashmap_test(), return EXIT_FAILURE);
    CHECK(tripplebuffer_tests(), return EXIT_FAILURE);

    return 0;
}