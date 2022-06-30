#include "stdsharp/containers/actions.h"
#include "test.h"

using namespace std;
using namespace std::ranges;
using namespace stdsharp;

template<typename T>
using dummy_predicate_t = bool(const T&);

template<typename T>
concept vec_req = requires(
    vector<T> v,
    iterator_t<vector<T>> iter,
    T value,
    dummy_predicate_t<T> dummy_predicate //
)
{
    stdsharp::actions::emplace(v, iter, std::move(value));
    stdsharp::actions::emplace_back(v, std::move(value));
    stdsharp::actions::emplace_front(v, std::move(value));

    stdsharp::actions::erase(v, value);
    stdsharp::actions::erase(v, iter);
    stdsharp::actions::erase(v, iter, iter);
    stdsharp::actions::erase_if(v, dummy_predicate);

    stdsharp::actions::pop_front(v);
    stdsharp::actions::pop_back(v);

    stdsharp::actions::resize(v, 0);
};

template<typename T>
concept set_req = requires(
    set<T> v,
    iterator_t<set<T>> iter,
    T value,
    dummy_predicate_t<T> dummy_predicate //
)
{
    stdsharp::actions::emplace(v, std::move(value));

    stdsharp::actions::erase(v, value);
    stdsharp::actions::erase(v, iter);
    stdsharp::actions::erase(v, iter, iter);
    stdsharp::actions::erase_if(v, dummy_predicate);
};

template<typename T>
concept unordered_map_req = requires(
    unordered_map<T, int> v,
    iterator_t<unordered_map<T, int>> iter,
    T value,
    dummy_predicate_t<pair<const T, int>> dummy_predicate //
)
{
    stdsharp::actions::emplace(v, std::move(value), 0);

    stdsharp::actions::erase(v, value);
    stdsharp::actions::erase(v, iter);
    stdsharp::actions::erase(v, iter, iter);
    stdsharp::actions::erase_if(v, dummy_predicate);
};

TEMPLATE_TEST_CASE( // NOLINT
    "vector actions", //
    "[containers][actions]",
    int,
    float //
)
{
    CAPTURE(type<TestType>());
    STATIC_REQUIRE(vec_req<TestType>);
}

TEMPLATE_TEST_CASE( // NOLINT
    "vector concept", //
    "[containers]",
    int,
    unique_ptr<int> //
)
{
    using vec = std::vector<TestType>;

    CAPTURE(type<vec>());
    STATIC_REQUIRE(containers::sequence_container<vec>);
    STATIC_REQUIRE(!containers::associative_container<vec>);
    STATIC_REQUIRE(!containers::unordered_associative_container<vec>);
}

TEMPLATE_TEST_CASE( // NOLINT
    "set actions", //
    "[containers][actions]",
    int,
    float //
)
{
    CAPTURE(type<TestType>());
    STATIC_REQUIRE(set_req<TestType>);
}

TEMPLATE_TEST_CASE( // NOLINT
    "set concept", //
    "[containers]",
    int,
    unique_ptr<int> //
)
{
    using set = std::set<TestType>;

    CAPTURE(type<set>());
    STATIC_REQUIRE(!containers::sequence_container<set>);
    STATIC_REQUIRE(containers::associative_container<set>);
    STATIC_REQUIRE(!containers::unordered_associative_container<set>);
}

TEMPLATE_TEST_CASE( // NOLINT
    "unordered map actions", //
    "[containers][actions]",
    int,
    float //
)
{
    CAPTURE(type<TestType>());
    STATIC_REQUIRE(unordered_map_req<TestType>);
}

TEMPLATE_TEST_CASE( // NOLINT
    "unordered map concept", //
    "[containers]",
    int,
    unique_ptr<int> //
)
{
    using map = unordered_map<int, TestType>;

    CAPTURE(type<map>());
    STATIC_REQUIRE(!containers::sequence_container<map>);
    STATIC_REQUIRE(!containers::associative_container<map>);
    STATIC_REQUIRE(containers::unordered_associative_container<map>);
}