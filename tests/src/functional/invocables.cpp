#include "stdsharp/functional/invocables.h"
#include "test.h"

SCENARIO("invocables", "[function][invocables]") // NOLINT
{
    struct f0
    {
        int operator()(int /*unused*/) const { return 0; }
    };

    struct f1
    {
        int operator()(char /*unused*/) const { return 1; }
    };

    struct f2
    {
        int operator()(const char* /*unused*/) const { return 2; }
    };

    struct f3
    {
        int operator()(const int* /*unused*/) const { return 3; }
    };

    using invocables = invocables<f0, f1, f2, f3>;

    STATIC_REQUIRE(std::constructible_from<invocables, f0, f1, f2, f3>);
    STATIC_REQUIRE(std::constructible_from<invocables, f0, f1, f2>);
    STATIC_REQUIRE(std::constructible_from<invocables, f0, f1>);
    STATIC_REQUIRE(std::constructible_from<invocables, f0>);
    STATIC_REQUIRE(std::constructible_from<invocables>);
    STATIC_REQUIRE(!std::invocable<invocables, int>);
    REQUIRE(invocables{}(static_cast<char*>(nullptr)) == 2);
    REQUIRE(invocables{}(static_cast<int*>(nullptr)) == 3);
}