#include "stdsharp/containers/containers.h"
#include "test.h"

using namespace containers;

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: container concept", //
    "[containers]",
    int,
    unique_ptr<int> //
)
{
    using vec = std::vector<TestType>;

    GIVEN(fmt::format("vector type {}", type<vec>()))
    {
        STATIC_REQUIRE(containers::sequence_container<vec>);
        STATIC_REQUIRE(!containers::associative_container<vec>);
        STATIC_REQUIRE(!containers::unordered_associative_container<vec>);
    }

    using set = std::set<TestType>;

    GIVEN(fmt::format("set type {}", type<set>()))
    {
        STATIC_REQUIRE(!containers::sequence_container<set>);
        STATIC_REQUIRE(containers::associative_container<set>);
        STATIC_REQUIRE(!containers::unordered_associative_container<set>);
    }

    using map = unordered_map<int, TestType>;

    GIVEN(fmt::format("map type {}", type<map>()))
    {
        STATIC_REQUIRE(!containers::sequence_container<map>);
        STATIC_REQUIRE(!containers::associative_container<map>);
        STATIC_REQUIRE(containers::unordered_associative_container<map>);
    }
}