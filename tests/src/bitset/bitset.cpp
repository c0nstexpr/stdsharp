#include "stdsharp/bitset/bitset.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("bitset iterator", "[bitset]")
{
    STATIC_REQUIRE(std::random_access_iterator<bitset_iterator<4>>);
    // STATIC_REQUIRE(std::random_access_iterator<bitset_const_iterator<4>>);
}

SCENARIO("bitset range", "[bitset]")
{
    // bitset<4> set{0b0101};
    // const auto& rng = bitset_rng(set);
}