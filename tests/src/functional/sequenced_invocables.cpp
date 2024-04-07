#include "stdsharp/functional/empty_invoke.h"
#include "stdsharp/functional/sequenced_invocables.h"
#include "test.h"

using namespace std;
using namespace stdsharp;

SCENARIO("sequenced invocables", "[functional]")
{
    [[maybe_unused]] sequenced_invocables fn{[](int) {}, [](array<int, 1>) {}, empty_invoke};

    using fn_t = decltype(fn);

    auto&& v = fn.get<2>();

    STATIC_REQUIRE(fn.size() == 3);
    STATIC_REQUIRE(std::invocable<fn_t, int>);
    STATIC_REQUIRE(std::invocable<fn_t&, int>);
    STATIC_REQUIRE(std::invocable<fn_t, array<int, 1>>);
    STATIC_REQUIRE(std::invocable<fn_t&, array<int, 1>>);
    STATIC_REQUIRE(std::invocable<fn_t, char*>);
    STATIC_REQUIRE(std::invocable<fn_t&, char*>);

}