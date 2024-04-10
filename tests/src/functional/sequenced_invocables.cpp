#include "stdsharp/functional/empty_invoke.h"
#include "stdsharp/functional/sequenced_invocables.h"
#include "test.h"

using namespace std;
using namespace stdsharp;

SCENARIO("sequenced invocables", "[functional]")
{
    sequenced_invocables fn{[](int i) { return i; }, [](array<int, 1>) {}, empty_invoke};
    using fn_t = decltype(fn);

    REQUIRE(fn.get<0>()(1) == 1);
    REQUIRE(fn.cget<0>()(1) == 1);
    REQUIRE(fn(1) == 1);

    STATIC_REQUIRE(fn.size() == 3);
    STATIC_REQUIRE(std::invocable<fn_t, int>);
    STATIC_REQUIRE(std::invocable<fn_t&, int>);
    STATIC_REQUIRE(std::invocable<fn_t, array<int, 1>>);
    STATIC_REQUIRE(std::invocable<fn_t&, array<int, 1>>);
    STATIC_REQUIRE(std::invocable<fn_t, char*>);
    STATIC_REQUIRE(std::invocable<fn_t&, char*>);
}