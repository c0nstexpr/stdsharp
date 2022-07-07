#include "stdsharp/type_traits/type_sequence.h"
#include "test.h"

using namespace type_traits;

using test_seq = type_sequence<int, float, char, unsigned, float>;

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: type sequence",
    "[type traits]",
    test_seq,
    type_sequence<> //
)
{
    STATIC_REQUIRE(default_initializable<TestType>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence get",
    "[type traits]",
    ((auto Index, typename Expect), Index, Expect),
    (0, int),
    (1, float),
    (2, char),
    (3, unsigned) //
)
{
    STATIC_REQUIRE(same_as<test_seq::get_t<Index>, Expect>);
}

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: type sequence invoke",
    "[type traits]",
    identity,
    decltype( //
        []( //
            const stdsharp::concepts::same_as_any<
                type_constant<int>,
                type_constant<char>,
                type_constant<unsigned>,
                type_constant<float>> auto //
        ) {} // clang-format off
    ) // clang-format on
)
{
    STATIC_REQUIRE(invocable<test_seq::invoke_fn<>, TestType>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence find",
    "[type traits]",
    ((typename T, auto Expect), T, Expect),
    (int, 0),
    (float, 1),
    (void, test_seq::size()) //
)
{
    STATIC_REQUIRE(test_seq::find(type_constant<T>{}) == Expect);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence count",
    "[type traits]",
    ((typename T, auto Expect), T, Expect),
    (int, 1),
    (float, 2),
    (void, 0) //
)
{
    STATIC_REQUIRE(test_seq::count(type_constant<T>{}) == Expect);
}

// clang-format on
SCENARIO("type sequence apply", "[type traits]") // NOLINT
{
    STATIC_REQUIRE(default_initializable<test_seq::apply_t<type_sequence>>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence indexed by sequence",
    "[type traits]",
    ((typename IndexSeq, typename Expect, auto V), IndexSeq, Expect, V),
    (regular_value_sequence<1, 2>, regular_type_sequence<float, char>, 0),
    (regular_value_sequence<2, 4>, regular_type_sequence<char, float>, 0) //
)
{
    STATIC_REQUIRE(same_as<test_seq::indexed_by_seq_t<IndexSeq>, Expect>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence append by sequence",
    "[type traits]",
    ( //
        (typename Seq, typename Expect, typename FrontExpect, auto V),
        Seq,
        Expect,
        FrontExpect,
        V // clang-format off
    ), // clang-format on
    ( //
        regular_type_sequence<void, double>,
        regular_type_sequence<int, float, char, unsigned, float, void, double>,
        (regular_type_sequence<void, double, int, float, char, unsigned, float>),
        0 // clang-format off
    ), // clang-format on
    ( //
        regular_type_sequence<long, void*>,
        regular_type_sequence<int, float, char, unsigned, float, long, void*>,
        (regular_type_sequence<long, void*, int, float, char, unsigned, float>),
        0 // clang-format off
    ) // clang-format on
)
{
    STATIC_REQUIRE(same_as<test_seq::append_by_seq_t<Seq>, Expect>);
    STATIC_REQUIRE(same_as<test_seq::append_front_by_seq_t<Seq>, FrontExpect>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence insert by sequence",
    "[type traits]",
    ((auto Index, typename Seq, typename Expect), Index, Seq, Expect),
    ( //
        3,
        regular_type_sequence<void, double>,
        regular_type_sequence<int, float, char, void, double, unsigned, float> // clang-format off
    ), // clang-format on
    ( //
        5,
        regular_type_sequence<long, void*>,
        regular_type_sequence<int, float, char, unsigned, float, long, void*> // clang-format off
    ) // clang-format on
)
{
    STATIC_REQUIRE(same_as<test_seq::insert_by_seq_t<Index, Seq>, Expect>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence remove at by sequence",
    "[type traits]",
    ((auto V, typename Seq, typename Expect), V, Seq, Expect),
    ( //
        0,
        regular_value_sequence<1, 2>,
        regular_type_sequence<int, unsigned, float> // clang-format off
    ), // clang-format on
    ( //
        0,
        regular_value_sequence<2, 4>,
        regular_type_sequence<int, float, unsigned> // clang-format off
    ) // clang-format on
)
{
    STATIC_REQUIRE(same_as<test_seq::remove_at_by_seq_t<Seq>, Expect>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: unique type sequence",
    "[type traits]",
    ((auto V, typename Seq, typename Expect), V, Seq, Expect),
    (0, type_sequence<>, type_sequence<>),
    (0, test_seq, type_sequence<int, float, char, unsigned>) //
)
{
    STATIC_REQUIRE( //
        same_as<
            typename Seq::template apply_t<type_traits::unique_type_sequence_t>,
            Expect // clang-format off
        > // clang-format on
    );
}