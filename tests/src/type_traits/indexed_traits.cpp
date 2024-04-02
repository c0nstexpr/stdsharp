#include "stdsharp/type_traits/indexed_traits.h"
#include "test.h"

using namespace std;
using namespace stdsharp;

SCENARIO("indexed traits", "[type traits]")
{
    GIVEN("type at")
    {
        STATIC_REQUIRE(same_as<type_at<0, int, float>, int>);
        STATIC_REQUIRE(same_as<type_at<1, int, float>, float>);
    }

    GIVEN("indexed values type")
    {
        using indexed_values = indexed_values<int, float>;

        THEN("type is constructible")
        {
            STATIC_REQUIRE(std::default_initializable<indexed_values>);
            STATIC_REQUIRE(std::constructible_from<indexed_values, int, float>);
            STATIC_REQUIRE(std::constructible_from<indexed_values, int, int>);
            STATIC_REQUIRE(std::constructible_from<indexed_values, float, int>);
        }

        [[maybe_unused]] indexed_values values{1, 23};

        THEN("indexed value getter type")
        {
            STATIC_REQUIRE(same_as<decltype(values.get<0>()), int&>);
            STATIC_REQUIRE(same_as<decltype(as_const(values).get<0>()), const int&>);
            STATIC_REQUIRE(same_as<decltype(std::move(values).get<0>()), int&&>);
            STATIC_REQUIRE(same_as<decltype(std::move(as_const(values)).get<0>()), const int&&>);
        }
    }
}