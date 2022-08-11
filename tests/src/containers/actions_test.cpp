#include "stdsharp/containers/actions.h"
#include "test.h"

using namespace containers;
using namespace functional;

template<typename Container>
concept erase_req = requires(
    Container container,
    typename Container::const_iterator iter,
    bool (&predicate)(typename Container::const_reference) //
)
{
    [](Container container)
    {
        if constexpr(associative_container<Container> || unordered_associative_container<Container>)
            [&](typename Container::key_type value) { actions::erase(container, value); };
        else if constexpr(sequence_container<Container>)
            [&](typename Container::value_type value) { actions::erase(container, value); };
    };
    actions::erase(container, iter);
    actions::erase(container, iter, iter);
    actions::erase_if(container, predicate);
};


TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: erase actions", //
    "[containers][actions]",
    vector<int>,
    set<int>,
    (map<int, int>),
    (unordered_map<int, int>) //
)
{
    CAPTURE(type<TestType>());
    STATIC_REQUIRE(erase_req<TestType>);
}

static_assert(!sequence_container<unordered_map<int, int>>);

template<typename Container>
concept emplace_req = requires(Container container)
{
    [](Container container)
    {
        if constexpr(associative_container<Container> || unordered_associative_container<Container>)
            [&] { actions::emplace(container, *container.cbegin()); };
        // else if constexpr(sequence_container<Container>)
        //     [&] { actions::emplace(container, container.cbegin(), container.front()); };
    };

    requires(!sequence_container<Container>) || requires
    {
        actions::emplace(container, container.cbegin(), container.front());
    };
};

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: emplace actions", //
    "[containers][actions]",
    vector<int>,
    set<int>(map<int, int>),
    (unordered_map<int, int>) //
)
{
    CAPTURE(type<TestType>());
    static_assert(emplace_req<TestType>);
}

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: emplace where actions", //
    "[containers][actions]",
    vector<int>,
    deque<int>,
    list<int> //
)
{
    CAPTURE(type<TestType>());

    STATIC_REQUIRE( //
        requires(
            TestType v,
            typename TestType::value_type value // clang-format off
        ) // clang-format on
        {
            actions::emplace_back(v, value);
            actions::emplace_front(v, value);
        } //
    );
}

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: pop where actions", //
    "[containers][actions]",
    vector<int>,
    deque<int>,
    list<int> //
)
{
    CAPTURE(type<TestType>());

    STATIC_REQUIRE( //
        requires(TestType v) //
        {
            actions::pop_back(v);
            actions::pop_front(v);
        } //
    );
}

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: resize actions", //
    "[containers][actions]",
    vector<int>,
    list<int> //
)
{
    CAPTURE(type<TestType>());

    STATIC_REQUIRE(requires(TestType v) { actions::resize(v, 5); });
}
