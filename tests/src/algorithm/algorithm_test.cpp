#include "stdsharp/algorithm/algorithm.h"
#include "test.h"

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
    (100, 50, 900),
    (3, 2, 3)
)
{
    GIVEN("given three values")
    {
        constexpr auto is_in_range = !(Value < Min) && !(Value > Max);

        THEN(fmt::format("value should {}between min-max", is_in_range ? "" : "not "))
        {
            STATIC_REQUIRE(is_between(Value, Min, Max) == is_in_range);
        }
    }
}