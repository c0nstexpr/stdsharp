#include <fmt/ranges.h>

#include "stdsharp/type_traits/value_sequence.h"
#include "test.h"

using namespace stdsharp;
using namespace type_traits;

using test_seq = value_sequence<0, 1, size_t{7}, 1, to_array("my literal")>;

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: value sequence",
    "[type traits]",
    test_seq,
    value_sequence<> //
)
{
    GIVEN(type<TestType>())
    THEN("type is constructible")
    STATIC_REQUIRE(default_initializable<TestType>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: value sequence get",
    "[type traits]",
    ((auto Index, auto Expect), Index, Expect),
    (0, 0),
    (1, 1),
    (2, 7),
    (3, 1) //
)
{
    GIVEN(type<test_seq>())
    AND_GIVEN(format("index: {}", Index))
    STATIC_REQUIRE(test_seq::get<Index>() == Expect);
}

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: value sequence invoke",
    "[type traits]",
    identity,
    decltype([](stdsharp::concepts::same_as_any<int, size_t, array<char, 11>> auto) {}))
{
    STATIC_REQUIRE(invocable<test_seq::invoke_fn<>, TestType>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: value sequence find",
    "[type traits]",
    ((auto V, auto Expect), V, Expect),
    (0, 0),
    (1, 1),
    ('!', test_seq::size()) //
)
{
    GIVEN(format("value: {}", V))
    THEN("found index should be expected")
    STATIC_REQUIRE(test_seq::find(V) == Expect);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: value sequence count",
    "[type traits]",
    ((auto V, auto Expect), V, Expect),
    (0, 1),
    (1, 2),
    ('?', 0) //
)
{
    GIVEN(format("value: {}", V))
    THEN("count should be expected")
    STATIC_REQUIRE(test_seq::count(V) == Expect);
}

// clang-format on
SCENARIO("apply_t", "[type traits]") // NOLINT
{
    GIVEN(format("template: {}", type<value_sequence<>>()))
    THEN("type that is applied sequence values should be constructible")
    STATIC_REQUIRE(default_initializable<test_seq::apply_t<value_sequence>>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: value sequence transform",
    "[type traits]",
    ((auto... Functor), Functor...),
    identity{},
    ( //
        [](const int v) mutable { return v + 1; },
        [](const int v) { return v + 42; },
        [](const size_t v) { return v + 42; },
        [](const int v) { return v + 6; },
        []<auto Size>(const array<char, Size>& str) { return str[0]; } // clang-format off
    ) // clang-format on
)
{
    GIVEN(format("Functors: {}", join({type<decltype(Functor)>()...}, ",")))
    THEN("type that is applied sequence values should be constructible")
    STATIC_REQUIRE(default_initializable<test_seq::template transform_t<Functor...>>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: value sequence indexed by sequence",
    "[type traits]",
    ((typename IndexSeq, typename Expect, auto V), IndexSeq, Expect, V),
    (regular_value_sequence<1, 2>, regular_value_sequence<1, size_t{7}>, 0),
    (regular_value_sequence<2, 4>, regular_value_sequence<size_t{7}, to_array("my literal")>, 0) //
)
{
    STATIC_REQUIRE(same_as<test_seq::indexed_by_seq_t<IndexSeq>, Expect>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: value sequence append by sequence",
    "[type traits]",
    ( //
        (typename Seq, typename Expect, typename FrontExpect, auto V),
        Seq,
        Expect,
        FrontExpect,
        V // clang-format off
    ), // clang-format on
    ( //
        regular_value_sequence<1, 2>,
        (regular_value_sequence<0, 1, size_t{7}, 1, to_array("my literal"), 1, 2>),
        regular_value_sequence<1, 2, 0, 1, size_t{7}, 1, to_array("my literal")>,
        0 // clang-format off
    ), // clang-format on
    ( //
        regular_value_sequence<2, 4>,
        (regular_value_sequence<0, 1, size_t{7}, 1, to_array("my literal"), 2, 4>),
        regular_value_sequence<2, 4, 0, 1, size_t{7}, 1, to_array("my literal")>,
        0 // clang-format off
    ) // clang-format on
)
{
    STATIC_REQUIRE(same_as<test_seq::append_by_seq_t<Seq>, Expect>);
    STATIC_REQUIRE(same_as<test_seq::append_front_by_seq_t<Seq>, FrontExpect>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: value sequence insert by sequence",
    "[type traits]",
    ((auto Index, typename Seq, typename Expect), Index, Seq, Expect),
    ( //
        3,
        regular_value_sequence<1, 2>,
        regular_value_sequence<0, 1, size_t{7}, 1, 2, 1, to_array("my literal")> // clang-format off
    ), // clang-format on
    ( //
        5,
        regular_value_sequence<2, 4>,
        regular_value_sequence<0, 1, size_t{7}, 1, to_array("my literal"), 2, 4> // clang-format off
    ) // clang-format on
)
{
    STATIC_REQUIRE(same_as<test_seq::insert_by_seq_t<Index, Seq>, Expect>);
}

//             // clang-format off
//             feature("remove_at_by_seq_t") = remove_at_by_seq_t_feat<test_seq>() | tuple<
//                 indexed_by_seq_t_test_params<
//                     regular_value_sequence<1, 2>,
//                     regular_value_sequence<0, 1, to_array("my literal")>
//                 >,
//                 indexed_by_seq_t_test_params<
//                     regular_value_sequence<2, 4>,
//                     regular_value_sequence<0, 1, 1>
//                 >
//             >{}; // clang-format on

//             // clang-format off
//             feature("unique_value_sequence_t") = []<typename Expect, auto... Values>(
//                 const unique_seq_t_test_params<Expect, Values...>
//             ) // clang-format on
//             {
//                 given("given values") = []
//                 {
//                     print( //
//                         format(
//                             "values: {}", // clang-format off
//                             fmt::join(std::tuple{Values...}, ", ")
//                         ) // clang-format on
//                     );

//                     then("use seq as unique_value_sequence_t template arg, "
//                          "type should be expected") = []
//                     {
//                         print(format("expected type: {}", reflection::type_name<Expect>()));
//                         using actual_t =
//                         stdsharp::type_traits::unique_value_sequence_t<Values...>;
//                         static_expect<_b(same_as<actual_t, Expect>)>() << //
//                             format("actual type: {}", reflection::type_name<actual_t>());
//                     };
//                 }; // clang-format off
//             } | tuple<
//                 unique_seq_t_test_params<regular_value_sequence<>>,
//                 unique_seq_t_test_params<
//                     regular_value_sequence<0, 10, 1, 5>,
//                     0, 10, 1, 5, 10, 1
//                 >
//             >{}; // clang-format on
//         };

//         return suite;
//     }
// }
