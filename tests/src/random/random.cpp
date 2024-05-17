#include "stdsharp/random/random.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("random concept", "[random]")
{
    STATIC_REQUIRE(seed_sequence<seed_seq>);
    STATIC_REQUIRE(random_number_engine<mt19937>);
    STATIC_REQUIRE(random_number_distribution<bernoulli_distribution>);
}