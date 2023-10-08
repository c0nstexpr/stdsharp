#include "stdsharp/containers/concepts.h"
#include "test.h"

using namespace std;
using namespace stdsharp;

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: container concept",
    "[containers]",
    int,
    unique_ptr<int>
)
{
    {
        using vec = vector<TestType>;

        GIVEN(format("vector type {}", type_id<vec>))
        {
            STATIC_REQUIRE(contiguous_container<vec>);
            STATIC_REQUIRE(!associative_container<vec>);
            STATIC_REQUIRE(!unordered_associative_container<vec>);
        }
    }

    {
        using arr = array<TestType, 5>;

        GIVEN(format("array type {}", type_id<arr>))
        {
            STATIC_REQUIRE(container<arr>);
            STATIC_REQUIRE(!associative_container<arr>);
            STATIC_REQUIRE(!unordered_associative_container<arr>);
        }
    }

    {
        using forward_list = forward_list<TestType>;

        GIVEN(format("forward list type {}", type_id<forward_list>))
        {
            STATIC_REQUIRE(container<forward_list>);
            STATIC_REQUIRE(!sequence_container<forward_list>);
            STATIC_REQUIRE(!associative_container<forward_list>);
            STATIC_REQUIRE(!unordered_associative_container<forward_list>);
        }
    }

    {
        using set = set<TestType>;

        GIVEN(format("set type {}", type_id<set>))
        {
            STATIC_REQUIRE(!sequence_container<set>);
            STATIC_REQUIRE(unique_associative_container<set>);
            STATIC_REQUIRE(!unordered_associative_container<set>);
        }
    }

    {
        using set = multiset<TestType>;

        GIVEN(format("set type {}", type_id<set>))
        {
            STATIC_REQUIRE(!sequence_container<set>);
            STATIC_REQUIRE(!unique_associative_container<set>);
            STATIC_REQUIRE(multikey_associative_container<set>);
            STATIC_REQUIRE(!unordered_associative_container<set>);
        }
    }

    {
        using map = unordered_map<int, TestType>;

        GIVEN(format("unordered map type {}", type_id<map>))
        {
            STATIC_REQUIRE(!sequence_container<map>);
            STATIC_REQUIRE(!associative_container<map>);
            STATIC_REQUIRE(unique_unordered_associative_container<map>);
        }
    }

    {
        using map = unordered_multimap<int, TestType>;

        GIVEN(format("unordered map type {}", type_id<map>))
        {
            STATIC_REQUIRE(!sequence_container<map>);
            STATIC_REQUIRE(!associative_container<map>);
            STATIC_REQUIRE(!unique_unordered_associative_container<map>);
            STATIC_REQUIRE(multikey_unordered_associative_container<map>);
        }
    }
}