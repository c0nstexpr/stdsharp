#include "stdsharp/algorithm/algorithm.h"
#include "test.h"

using namespace std;
using namespace fmt;
using namespace stdsharp;

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: set if",
    "[algorithm]",
    ((auto First, auto Second), First, Second),
    (1, 2),
    (2, 1)
)
{
    GIVEN("given two values")
    {
        THEN("base on comparison result to set value")
        {
            constexpr auto order = partial_order(Second, First);
            constexpr auto greater = order > 0; // NOLINT(*-use-nullptr)
            constexpr auto less = order < 0; // NOLINT(*-use-nullptr)
            constexpr auto f = [](const auto expect, const auto& func) consteval
            {
                auto first = First;
                return expect == (func(first, Second) == Second);
            };

            STATIC_REQUIRE(f(greater, set_if_greater));
            STATIC_REQUIRE(f(less, set_if_less));
        }
    }
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: is between",
    "[algorithm]",
    ((auto Value, auto Min, auto Max), Value, Min, Max),
    (1, 1, 2),
    (4, 2, 4),
    (3, 5, 10),
    (10, -1, 9),
    (3, 3, 5),
    (100, 50, 900),
    (3, 2, 3)
)
{
    GIVEN("given three values")
    {
        static constexpr auto v = Value;
        static constexpr auto min = Min;
        static constexpr auto max = Max;

        THEN("is between result should be correct")
        {
            STATIC_REQUIRE(is_between(v, min, max) == ((v >= min) && (v <= max)));
        }
    }
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: strict compare",
    "[algorithm]",
    ((auto Left, auto Right, auto GetCmp), Left, Right, GetCmp),
    (array{3, 4}, array{1, 2}, [] { return partial_ordering::greater; }),
    (array{1, 2}, array{3, 2}, [] { return partial_ordering::less; }),
    (array{1, 2}, array{1, 2}, [] { return partial_ordering::equivalent; }),
    (array{0, 3}, array{1, 2}, [] { return partial_ordering::unordered; }),
    (array{2, 2}, array{1, 2}, [] { return partial_ordering::greater; })
)
{
    GIVEN("given three values")
    {
        THEN("is between result should be correct")
        {
            STATIC_REQUIRE(strict_compare(Left, Right) == GetCmp());
        }
    }
}