#include "stdsharp/utility/value_wrapper.h"
#include "test.h"

using namespace std;
using namespace stdsharp;

SCENARIO("value_wrapper", "[value_wrapper]") // NOLINT
{
    STATIC_REQUIRE(default_initializable<value_wrapper<int>>);
    STATIC_REQUIRE(constructible_from<value_wrapper<int>, int>);
    STATIC_REQUIRE(movable<value_wrapper<int>>);
    STATIC_REQUIRE(copyable<value_wrapper<int>>);
    STATIC_REQUIRE(swappable<value_wrapper<int>>);

    [[maybe_unused]] value_wrapper<int> wrapper{};

    STATIC_REQUIRE(same_as<decltype(wrapper.get()), int&>);
    STATIC_REQUIRE(same_as<decltype(std::move(wrapper).get()), int&&>);
    STATIC_REQUIRE(same_as<decltype(wrapper.cget()), const int&>);
    STATIC_REQUIRE(same_as<decltype(std::move(wrapper).cget()), const int&&>);
}