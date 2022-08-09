#include "stdsharp/containers/actions.h"
#include "test.h"

using namespace std::ranges;

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
    actions::emplace(v, std::move(value));
};

template<typename T>
concept unordered_map_req = requires(
    unordered_map<T, int> v,
    iterator_t<unordered_map<T, int>> iter,
    T value,
    dummy_predicate_t<typename decltype(v)::value_type> dummy_predicate //
)
{
    actions::emplace(v, std::move(value), 0);
};

template<typename T>
struct get_value_type : std::type_identity<typename T::value_type>
{
};

template<typename T>
    requires(containers::associative_container<T> || containers::unordered_associative_container<T>)
struct get_value_type<T> : std::type_identity<typename T::key_type>
{
};

template<typename T>
using get_value_t = typename get_value_type<T>::type;

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
    STATIC_REQUIRE( //
        requires(
            TestType v,
            typename TestType::const_iterator iter,
            get_value_t<TestType> value,
            dummy_predicate_t<decltype(value)> dummy_predicate // clang-format off
        )
        { // clang-format on
            actions::erase(v, value);
            actions::erase(v, iter);
            actions::erase(v, iter, iter);
        } //
    );
}
