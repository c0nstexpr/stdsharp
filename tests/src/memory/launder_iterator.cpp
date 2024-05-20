#include "stdsharp/memory/launder_iterator.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("launder iterator", "[memory]")
{
    STATIC_REQUIRE(contiguous_iterator<launder_iterator<int*>>);
}