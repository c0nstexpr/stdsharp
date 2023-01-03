#include "stdsharp/containers/concepts.h"
#include "test.h"

using namespace std;
using namespace fmt;
using namespace stdsharp;

static_assert(associative_container<set<unique_ptr<int>>>);

// static_assert(unique_associative_container<set<unique_ptr<int>>>);

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: container concept",
    "[containers]",
    int,
    unique_ptr<int>
)
{
    {
        using vec = vector<TestType>;

        GIVEN(fmt::format("vector type {}", type<vec>()))
        {
            STATIC_REQUIRE(contiguous_container<vec>);
            STATIC_REQUIRE(!associative_container<vec>);
            STATIC_REQUIRE(!unordered_associative_container<vec>);
        }
    }

    {
        using arr = array<TestType, 5>;

        GIVEN(fmt::format("array type {}", type<arr>()))
        {
            STATIC_REQUIRE(container<arr>);
            STATIC_REQUIRE(!associative_container<arr>);
            STATIC_REQUIRE(!unordered_associative_container<arr>);
        }
    }

    {
        using forward_list = forward_list<TestType>;

        GIVEN(fmt::format("forward list type {}", type<forward_list>()))
        {
            STATIC_REQUIRE(container<forward_list>);
            STATIC_REQUIRE(!sequence_container<forward_list>);
            STATIC_REQUIRE(!associative_container<forward_list>);
            STATIC_REQUIRE(!unordered_associative_container<forward_list>);
        }
    }

    // {
    //     using set = set<TestType>;

    //     GIVEN(fmt::format("set type {}", type<set>()))
    //     {
    //         STATIC_REQUIRE(!sequence_container<set>);
    //         STATIC_REQUIRE(unique_associative_container<set>);
    //         STATIC_REQUIRE(!unordered_associative_container<set>);
    //     }
    // }

    // {
    //     using set = multiset<TestType>;


    //     GIVEN(fmt::format("set type {}", type<set>()))
    //     {
    //         STATIC_REQUIRE(!sequence_container<set>);
    //         STATIC_REQUIRE(!unique_associative_container<set>);
    //         STATIC_REQUIRE(multikey_associative_container<set>);
    //         STATIC_REQUIRE(!unordered_associative_container<set>);
    //     }
    // }

    // {
    //     using map = unordered_map<int, TestType>;

    //     GIVEN(fmt::format("unordered map type {}", type<map>()))
    //     {
    //         STATIC_REQUIRE(!sequence_container<map>);
    //         STATIC_REQUIRE(!associative_container<map>);
    //         STATIC_REQUIRE(unique_unordered_associative_container<map>);
    //     }
    // }

    // {
    //     using map = unordered_multimap<int, TestType>;

    //     GIVEN(fmt::format("unordered map type {}", type<map>()))
    //     {
    //         STATIC_REQUIRE(!sequence_container<map>);
    //         STATIC_REQUIRE(!associative_container<map>);
    //         STATIC_REQUIRE(!unique_unordered_associative_container<map>);
    //         STATIC_REQUIRE(multikey_unordered_associative_container<map>);
    //     }
    // }
}