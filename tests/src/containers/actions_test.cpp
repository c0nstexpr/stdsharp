#include "stdsharp/containers/actions.h"
#include "test.h"

using namespace containers;

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

    using get_element_type = typename decltype(
        []()
        {
            if constexpr(
                associative_container<TestType> || unordered_associative_container<TestType> //
            )
                return std::type_identity<typename TestType::key_type>{};
            else
                return std::type_identity<typename TestType::value_type>{};
        }() // clang-format off
    )::type; // clang-format on

    using dummy_predicate = bool(typename TestType::const_reference);

    STATIC_REQUIRE( //
        requires(
            TestType container,
            typename TestType::const_iterator iter,
            get_element_type element,
            dummy_predicate predicate // clang-format off
        )
        { // clang-format on
            actions::erase(container, element);
            actions::erase(container, iter);
            actions::erase(container, iter, iter);
            actions::erase_if(container, predicate);
        } //
    );
}

constexpr auto foo()
{
    using TestType = set<int>;

    using value_t = typename TestType::value_type;

    return container<TestType> || requires(TestType v, value_t value)
    {
        actions::emplace(v, v.cbegin(), value);
    };
};

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: emplace actions", //
    "[containers][actions]",
    vector<int>,
    set<int>,
    (map<int, int>),
    (unordered_map<int, int>) //
)
{
    using value_t = typename TestType::value_type;

    CAPTURE(type<TestType>());

    // STATIC_REQUIRE( //
    //     (!sequence_container<TestType>) ||
    //     requires(TestType v, value_t value) { actions::emplace(v, v.cbegin(), value); } //
    // );

    STATIC_REQUIRE( //
        !(associative_container<TestType> || unordered_associative_container<TestType>) ||
        requires(TestType v, value_t value) { actions::emplace(v, value); } //
    );
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
