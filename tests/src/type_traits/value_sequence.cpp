#include "stdsharp/functional/invocables.h"
#include "stdsharp/functional/sequenced_invocables.h"
#include "stdsharp/type_traits/value_sequence.h"

#include "test.h"

using namespace std;
using namespace stdsharp;

using test_seq = value_sequence<0, 1, size_t{7}, 1, to_array("my literal")>;

// TEMPLATE_TEST_CASE( // NOLINT
//     "Scenario: value sequence default initializable",
//     "[type traits]",
//     test_seq,
//     value_sequence<>
// )
// {
//     STATIC_REQUIRE(default_initializable<TestType>);
// }

// TEMPLATE_TEST_CASE_SIG( // NOLINT
//     "Scenario: value sequence get",
//     "[type traits]",
//     ((auto Index, auto Expect), Index, Expect),
//     (0, 0),
//     (1, 1),
//     (2, 7),
//     (3, 1)
// )
// {
//     STATIC_REQUIRE(get<Index>(test_seq{}) == Expect);
// }

// TEMPLATE_TEST_CASE( // NOLINT
//     "Scenario: value sequence invoke",
//     "[type traits]",
//     identity,
//     decltype([](stdsharp::same_as_any<int, size_t, array<char, 11>> auto) {})
// )
// {
//     STATIC_REQUIRE(invocable<test_seq::invoke_fn<>, TestType>);
// }

// TEMPLATE_TEST_CASE_SIG( // NOLINT
//     "Scenario: value sequence find",
//     "[type traits]",
//     ((auto V, auto Expect), V, Expect),
//     (0, 0),
//     (1, 1),
//     ('!', test_seq::size())
// )
// {
//     STATIC_REQUIRE(test_seq::find(V) == Expect);
// }

// namespace // Escape Catch2 special characters like '[' and ']'
// {
//     inline constexpr sequenced_invocables find_if_functor_3{
//         [](const size_t v) { return v == 7; },
//         always_false
//     };
// }

// TEMPLATE_TEST_CASE_SIG( // NOLINT
//     "Scenario: value sequence find if",
//     "[type traits]",
//     ((auto Fn, auto Expect), Fn, Expect),
//     (always_true, 0),
//     (always_false, test_seq::size()),
//     (find_if_functor_3, 2)
// )
// {
//     STATIC_REQUIRE(test_seq::find_if(Fn) == Expect);
// }

// TEMPLATE_TEST_CASE_SIG( // NOLINT
//     "Scenario: value sequence count",
//     "[type traits]",
//     ((auto V, auto Expect), V, Expect),
//     (0, 1),
//     (1, 2),
//     ('?', 0)
// )
// {
//     STATIC_REQUIRE(test_seq::count(V) == Expect);
// }

// namespace // Escape Catch2 special characters like '[' and ']'
// {
//     inline constexpr sequenced_invocables count_if_functor_3{
//         [](const size_t) { return true; },
//         always_false
//     };
// }

// TEMPLATE_TEST_CASE_SIG( // NOLINT
//     "Scenario: value sequence count if",
//     "[type traits]",
//     ((auto Fn, auto Expect), Fn, Expect),
//     (always_true, test_seq::size()),
//     (always_false, 0),
//     (count_if_functor_3, 4)
// )
// {
//     STATIC_REQUIRE(test_seq::count_if(Fn) == Expect);
// }

// SCENARIO("apply_t", "[type traits]") // NOLINT
// {
//     STATIC_REQUIRE(default_initializable<test_seq::apply_t<value_sequence>>);
// }

// namespace // Escape Catch2 special characters like '[' and ']'
// {
//     constexpr auto transform_functor_1 = [](const int v) mutable { return v + 1; };
//     constexpr auto transform_functor_2 = [](const int v) { return v + 42; };
//     constexpr auto transform_functor_3 = [](const size_t v) { return v + 42; };
//     constexpr auto transform_functor_4 = [](const int v) { return v + 6; };
//     constexpr auto transform_functor_5 = [](const array<char, 11>& str) { return str[0]; };
// }

// TEMPLATE_TEST_CASE_SIG( // NOLINT
//     "Scenario: value sequence transform",
//     "[type traits]",
//     ((auto... Functor), Functor...),
//     identity{},
//     ( //
//         transform_functor_1,
//         transform_functor_2,
//         transform_functor_3,
//         transform_functor_4,
//         transform_functor_5
//     )
// )
// {
//     STATIC_REQUIRE(default_initializable<test_seq::template transform_t<Functor...>>);
// }

// TEMPLATE_TEST_CASE_SIG( // NOLINT
//     "Scenario: value sequence at",
//     "[type traits]",
//     ((typename Expect, auto... V), Expect, V...),
//     (regular_value_sequence<1, size_t{7}>, 1, 2),
//     (regular_value_sequence<size_t{7}, to_array("my literal")>, 2, 4)
// )
// {
//     STATIC_REQUIRE(same_as<test_seq::at_t<V...>, Expect>);
// }

// TEMPLATE_TEST_CASE_SIG( // NOLINT
//     "Scenario: value sequence append",
//     "[type traits]",
//     ( //
//         (typename Expect, typename FrontExpect, auto... V),
//         Expect,
//         FrontExpect,
//         V...
//     ),
//     ( //
//         (regular_value_sequence<0, 1, size_t{7}, 1, to_array("my literal"), 1, 2>),
//         regular_value_sequence<1, 2, 0, 1, size_t{7}, 1, to_array("my literal")>,
//         1,
//         2
//     ),
//     ( //
//         (regular_value_sequence<0, 1, size_t{7}, 1, to_array("my literal"), 2, 4>),
//         regular_value_sequence<2, 4, 0, 1, size_t{7}, 1, to_array("my literal")>,
//         2,
//         4
//     )
// )
// {
//     STATIC_REQUIRE(same_as<test_seq::append_t<V...>, Expect>);
//     STATIC_REQUIRE(same_as<test_seq::append_front_t<V...>, FrontExpect>);
// }

// TEMPLATE_TEST_CASE_SIG( // NOLINT
//     "Scenario: value sequence insert",
//     "[type traits]",
//     ((auto Index, typename Expect, auto... Element), Index, Expect, Element...),
//     ( //
//         3,
//         regular_value_sequence<0, 1, size_t{7}, 1, 2, 1, to_array("my literal")>,
//         1,
//         2
//     ),
//     ( //
//         5,
//         regular_value_sequence<0, 1, size_t{7}, 1, to_array("my literal"), 2, 4>,
//         2,
//         4
//     )
// )
// {
//     STATIC_REQUIRE(same_as<test_seq::insert_t<Index, Element...>, Expect>);
// }

// TEMPLATE_TEST_CASE_SIG( // NOLINT
//     "Scenario: value sequence remove at",
//     "[type traits]",
//     ((typename Expect, auto... I), Expect, I...),
//     ( //
//         regular_value_sequence<0, 1, to_array("my literal")>,
//         1,
//         2
//     ),
//     ( //
//         regular_value_sequence<0, 1, 1>,
//         2,
//         4
//     )
// )
// {
//     STATIC_REQUIRE(same_as<test_seq::remove_at_t<I...>, Expect>);
// }

// TEMPLATE_TEST_CASE_SIG( // NOLINT
//     "Scenario: unique value sequence",
//     "[type traits]",
//     ((auto V, typename Seq, typename Expect), V, Seq, Expect),
//     (0, regular_value_sequence<>, regular_value_sequence<>),
//     (0, regular_value_sequence<0, 10, 1, 5, 10, 1>, regular_value_sequence<0, 10, 1, 5>) //
// )
// {
//     STATIC_REQUIRE( //
//         same_as<
//             typename to_value_sequence<Seq>:: //
//             template apply_t<unique_value_sequence>,
//             Expect
//         >
//     );
// }

// TEMPLATE_TEST_CASE_SIG( // NOLINT
//     "Scenario: reverse value sequence",
//     "[type traits]",
//     ((auto V, typename Seq, typename Expect), V, Seq, Expect),
//     (0, regular_value_sequence<>, regular_value_sequence<>),
//     (0, regular_value_sequence<1>, regular_value_sequence<1>),
//     (0, regular_value_sequence<1, 2, 3, 4>, regular_value_sequence<4, 3, 2, 1>),
//     (0, regular_value_sequence<1, 2>, regular_value_sequence<2, 1>) //
// )
// {
//     STATIC_REQUIRE( //
//         same_as<
//             typename to_value_sequence<Seq>:: //
//             template apply_t<reverse_value_sequence>,
//             Expect
//         >
//     );
// }