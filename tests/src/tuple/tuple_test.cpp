#include "stdsharp/tuple/tuple.h"
#include "test.h"

using namespace stdsharp;
using namespace stdsharp::concepts;

SCENARIO("get, [tuple]") // NOLINT
{
    using my_tuple_t = tuple<int, char, float>;

    STATIC_REQUIRE(decay_same_as<get_t<0, my_tuple_t>, int>);
    STATIC_REQUIRE(decay_same_as<get_t<1, my_tuple_t>, char>);
    STATIC_REQUIRE(decay_same_as<get_t<2, my_tuple_t>, float>);
} // clang-format on