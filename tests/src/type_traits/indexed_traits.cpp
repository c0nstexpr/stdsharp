#include "stdsharp/type_traits/indexed_traits.h"
#include "test.h"

using namespace std;
using namespace stdsharp;

SCENARIO("indexed traits", "[type traits]")
{
    GIVEN("indexed types")
    {
        using indexed_types = indexed_types<int, float>;

        THEN("get indexed type")
        {
            STATIC_REQUIRE(indexed_types{}.get_type(index_constant<0>{}) == type_constant<int>{});
            STATIC_REQUIRE(same_as<tuple_element_t<0, indexed_types>, int>);

            STATIC_REQUIRE(indexed_types{}.get_type(index_constant<1>{}) == type_constant<float>{});
            STATIC_REQUIRE(same_as<tuple_element_t<1, indexed_types>, float>);
        }
    }

    GIVEN("indexed values")
    {
        using indexed_values = indexed_values<empty_t, float>;

        [[maybe_unused]] indexed_values values{};

        THEN("get indexed value")
        {
            STATIC_REQUIRE(same_as<decltype(values.get<0>()), empty_t&>);
            STATIC_REQUIRE(same_as<decltype(as_const(values).get<0>()), const empty_t&>);
            STATIC_REQUIRE(same_as<decltype(std::move(values).get<0>()), empty_t&&>);
            STATIC_REQUIRE(same_as<decltype(std::move(as_const(values)).get<0>()), const empty_t&&>);
        }
    }
}