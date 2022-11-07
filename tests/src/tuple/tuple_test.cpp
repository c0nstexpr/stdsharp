#include "test.h"
#include "stdsharp/tuple/tuple.h"

using namespace stdsharp;

using my_tuple_t = std::tuple<int, char, float>;

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: get",
    "[tuple]",
    ((auto Index, typename T), Index, T),
    (0, int),
    (1, char),
    (2, float)
)
{
    STATIC_REQUIRE(decay_same_as<get_t<Index, my_tuple_t>, T>);
}