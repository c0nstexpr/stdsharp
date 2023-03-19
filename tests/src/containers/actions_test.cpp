#include "stdsharp/containers/actions.h"
#include "test.h"

using namespace std;
using namespace fmt;
using namespace stdsharp;

template<typename Container>
    requires associative_like_container<Container>
consteval auto erase_req_f()
{
    return requires(Container container) //
    {
        actions::cpo::erase(container, declval<typename Container::key_type>()); //
    };
}

template<sequence_container Container>
consteval auto erase_req_f()
{
    return requires(Container container) //
    {
        actions::cpo::erase(container, declval<typename Container::value_type>()); //
    };
}

template<typename Container>
concept erase_req = requires(
    Container container,
    typename Container::const_iterator iter,
    bool (&predicate)(typename Container::const_reference)
) //
{
    requires erase_req_f<Container>();
    actions::cpo::erase(container, iter);
    actions::cpo::erase(container, iter, iter);
    actions::cpo::erase_if(container, predicate);
};

void foo(set<int> v, const set<int>::const_iterator iter, bool (&predicate)(int))
{
    actions::cpo::erase(v, iter);
    actions::cpo::erase(v, iter, iter);

    using fn = sequenced_invocables<
        actions::cpo::cpo_impl::details::adl_erase_if_fn,
        actions::cpo::cpo_impl::details::default_erase_if_fn>;

    constexpr auto f = []<template<typename...> typename Inner, typename... Func, typename... Args>
        requires ::std::derived_from<Inner<Func...>, invocables<Func...>>(
            const Inner<Func...>&,
            Args && ... args
        )
    {

    };

    static_assert(requires(const fn& f) { f(f, v, predicate); });

    constexpr auto res = details::invocables_req<const fn, decltype(v), decltype(predicate)>(){};
    // constexpr auto i0 = res[0];
    // constexpr auto i1 = res[1];
    // constexpr auto i = ::std::ranges::lower_bound(res, expr_req::well_formed) - res.cbegin();
    // fn{}(v, predicate);
}

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: erase actions",
    "[containers][actions]",
    vector<int>,
    set<int>
    // (map<int, int>),
    // (unordered_map<int, int>)
)
{
    CAPTURE(type<TestType>());
    // STATIC_REQUIRE(erase_req<TestType>);
}

template<typename Container>
    requires associative_container<Container> || unordered_associative_container<Container>
consteval auto emplace_req_f()
{
    return requires(Container container) { actions::emplace(container, *container.cbegin()); };
}

template<sequence_container Container>
consteval auto emplace_req_f()
{
    return requires(Container container, typename Container::value_type v) //
    {
        actions::emplace(container, container.cbegin(), v); //
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
    (unordered_map<int, int>)
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
    list<int>
)
{
    CAPTURE(type<TestType>());

    STATIC_REQUIRE( //
        requires(TestType v, typename TestType::value_type value) //
        {
            actions::emplace_back(v, value);
            actions::emplace_front(v, value);
        }
    );
}

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: pop where actions",
    "[containers][actions]",
    vector<int>,
    deque<int>,
    list<int>
)
{
    CAPTURE(type<TestType>());

    STATIC_REQUIRE( //
        requires(TestType v) //
        {
            actions::pop_back(v);
            actions::pop_front(v);
        }
    );
}

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: resize actions",
    "[containers][actions]",
    vector<int>,
    list<int>
)
{
    CAPTURE(type<TestType>());

    STATIC_REQUIRE(requires(TestType v) { actions::resize(v, 5); });
}
