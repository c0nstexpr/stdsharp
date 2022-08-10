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
    {
        using vec = std::vector<TestType>;

        GIVEN(fmt::format("vector type {}", type<vec>()))
        {
            STATIC_REQUIRE(containers::sequence_container<vec>);
            STATIC_REQUIRE(!containers::associative_container<vec>);
            STATIC_REQUIRE(!containers::unordered_associative_container<vec>);
        }
    }

    {
        using forward_list = std::forward_list<TestType>;

        GIVEN(fmt::format("forward list type {}", type<forward_list>()))
        {
            STATIC_REQUIRE(containers::container<forward_list>);
            STATIC_REQUIRE(!containers::sequence_container<forward_list>);
            STATIC_REQUIRE(!containers::associative_container<forward_list>);
            STATIC_REQUIRE(!containers::unordered_associative_container<forward_list>);
        }
    }

    {
        using set = std::set<TestType>;

        GIVEN(fmt::format("set type {}", type<set>()))
        {
            STATIC_REQUIRE(!containers::sequence_container<set>);
            STATIC_REQUIRE(containers::unique_associative_container<set>);
            STATIC_REQUIRE(!containers::unordered_associative_container<set>);
        }
    }

    {
        using set = std::multiset<TestType>;

        GIVEN(fmt::format("set type {}", type<set>()))
        {
            STATIC_REQUIRE(!containers::sequence_container<set>);
            STATIC_REQUIRE(!containers::unique_associative_container<set>);
            STATIC_REQUIRE(containers::multikey_associative_container<set>);
            STATIC_REQUIRE(!containers::unordered_associative_container<set>);
        }
    }

    {
        using map = unordered_map<int, TestType>;

        GIVEN(fmt::format("unordered map type {}", type<map>()))
        {
            STATIC_REQUIRE(!containers::sequence_container<map>);
            STATIC_REQUIRE(!containers::associative_container<map>);
            STATIC_REQUIRE(containers::unique_unordered_associative_container<map>);
        }
    }

    {
        using map = unordered_multimap<int, TestType>;

        GIVEN(fmt::format("unordered map type {}", type<map>()))
        {
            STATIC_REQUIRE(!containers::sequence_container<map>);
            STATIC_REQUIRE(!containers::associative_container<map>);
            STATIC_REQUIRE(!containers::unique_unordered_associative_container<map>);
            STATIC_REQUIRE(containers::multikey_unordered_associative_container<map>);
        }
    }
}