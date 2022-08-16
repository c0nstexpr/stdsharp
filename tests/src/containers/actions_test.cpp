#include "stdsharp/containers/actions.h"
#include "test.h"

using namespace containers;
using namespace functional;

template<typename Container>
    requires associative_container<Container> || unordered_associative_container<Container>
constexpr auto erase_req_f()
{
    return requires(Container container)
    {
        actions::erase(container, declval<typename Container::key_type>());
    };
}

template<sequence_container Container>
constexpr auto erase_req_f()
{
    return requires(Container container)
    {
        actions::erase(container, declval<typename Container::value_type>());
    };
}

template<typename Container>
concept erase_req = erase_req_f<Container>() && requires(
    Container container,
    typename Container::const_iterator iter,
    bool (&predicate)(typename Container::const_reference) //
)
{
    actions::erase(container, iter);
    actions::erase(container, iter, iter);
    actions::erase_if(container, predicate);
};


TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: erase actions",
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

template<typename Container>
    requires associative_container<Container> || unordered_associative_container<Container>
constexpr auto emplace_req_f()
{
    return requires(Container container) { actions::emplace(container, *container.cbegin()); };
}

template<sequence_container Container>
constexpr auto emplace_req_f()
{
    return requires(Container container, typename Container::value_type v)
    {
        actions::emplace(container, container.cbegin(), v);
    };
}

template<typename Container>
concept emplace_req = emplace_req_f<Container>();

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: emplace actions",
    "[containers][actions]",
    vector<int>,
    set<int>,
    (map<int, int>),
    (unordered_map<int, int>) //
)
{
    CAPTURE(type<TestType>());
    STATIC_REQUIRE(emplace_req<TestType>);
}

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: emplace where actions",
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
    "Scenario: pop where actions",
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
    "Scenario: resize actions",
    "[containers][actions]",
    vector<int>,
    list<int> //
)
{
    CAPTURE(type<TestType>());

    STATIC_REQUIRE(requires(TestType v) { actions::resize(v, 5); });
}
