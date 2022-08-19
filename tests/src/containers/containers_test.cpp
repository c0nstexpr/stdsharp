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
        using vec = vector<TestType>;

        GIVEN(format("vector type {}", type<vec>()))
        {
            STATIC_REQUIRE(contiguous_container<vec>);
            STATIC_REQUIRE(!associative_container<vec>);
            STATIC_REQUIRE(!unordered_associative_container<vec>);
        }
    }

    {
        using arr = array<TestType, 5>;

        GIVEN(format("array type {}", type<arr>()))
        {
            STATIC_REQUIRE(container<arr>);
            STATIC_REQUIRE(!associative_container<arr>);
            STATIC_REQUIRE(!unordered_associative_container<arr>);
        }
    }

    {
        using forward_list = forward_list<TestType>;

        GIVEN(format("forward list type {}", type<forward_list>()))
        {
            STATIC_REQUIRE(container<forward_list>);
            STATIC_REQUIRE(!sequence_container<forward_list>);
            STATIC_REQUIRE(!associative_container<forward_list>);
            STATIC_REQUIRE(!unordered_associative_container<forward_list>);
        }
    }

    {
        using set = set<TestType>;

        GIVEN(format("set type {}", type<set>()))
        {
            STATIC_REQUIRE(!sequence_container<set>);
            STATIC_REQUIRE(unique_associative_container<set>);
            STATIC_REQUIRE(!unordered_associative_container<set>);
        }
    }

    {
        using set = multiset<TestType>;

        GIVEN(format("set type {}", type<set>()))
        {
            STATIC_REQUIRE(!sequence_container<set>);
            STATIC_REQUIRE(!unique_associative_container<set>);
            STATIC_REQUIRE(multikey_associative_container<set>);
            STATIC_REQUIRE(!unordered_associative_container<set>);
        }
    }

    {
        using map = unordered_map<int, TestType>;

        GIVEN(format("unordered map type {}", type<map>()))
        {
            STATIC_REQUIRE(!sequence_container<map>);
            STATIC_REQUIRE(!associative_container<map>);
            STATIC_REQUIRE(unique_unordered_associative_container<map>);
        }
    }

    {
        using map = unordered_multimap<int, TestType>;

        GIVEN(format("unordered map type {}", type<map>()))
        {
            STATIC_REQUIRE(!sequence_container<map>);
            STATIC_REQUIRE(!associative_container<map>);
            STATIC_REQUIRE(!unique_unordered_associative_container<map>);
            STATIC_REQUIRE(multikey_unordered_associative_container<map>);
        }
    }
}