#include "stdsharp/lazy.h"
#include "test.h"

SCENARIO("lazy", "[lazy]") // NOLINT
{
    GIVEN("a lazy value")
    {
        stdsharp::lazy lazy{[]() { return string{"foo"}; }};

        WHEN("get value")
        {
            auto value = lazy.get();

            THEN("value is 1") { REQUIRE(value == "foo"); }
        }
    }

    GIVEN("constexpr test")
    {
        constexpr auto value = 43;

        STATIC_REQUIRE(
            []
            {
                stdsharp::lazy lazy{[]() { return value; }};
                return lazy.get();
            }() == value
        );
    }
}