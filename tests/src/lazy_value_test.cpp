#include "stdsharp/lazy.h"
#include "test.h"

SCENARIO("lazy value", "[lazy value]") // NOLINT
{
    GIVEN("a lazy value")
    {
        stdsharp::lazy_value lazy{[]() { return 1; }};

        WHEN("get value")
        {
            auto value = lazy.get();

            THEN("value is 1") { REQUIRE(value == 1); }
        }
    }

    GIVEN("constexpr test")
    {
        constexpr auto value = 43;

        STATIC_REQUIRE(
            []
            {
                stdsharp::lazy_value lazy{[]() { return value; }};
                return lazy.cget();
            }() == value
        );
    }
}