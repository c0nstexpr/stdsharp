#include "stdsharp/type_traits/type_sequence.h"
#include "test.h"

using test_seq = type_sequence<int, float, char, unsigned, float>;

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: type sequence",
    "[type traits][type sequence]",
    test_seq,
    type_sequence<>
)
{
    STATIC_REQUIRE(default_initializable<TestType>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence type",
    "[type traits][type sequence]",
    ((auto Index, typename Expect), Index, Expect),
    (0, int),
    (1, float),
    (2, char),
    (3, unsigned)
)
{
    STATIC_REQUIRE(same_as<test_seq::type<Index>, Expect>);
}

SCENARIO("type sequence apply", "[type traits][type sequence]") // NOLINT
{
    STATIC_REQUIRE(default_initializable<test_seq::apply_t<type_sequence>>);
}

namespace
{
    inline constexpr auto type_sequence_invoker =
        []<typename T>(const basic_type_constant<T>) consteval {};
}

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: type sequence invoke",
    "[type traits][type sequence]",
    identity,
    decltype(type_sequence_invoker)
)
{
    STATIC_REQUIRE(invocable<test_seq::invoke_fn<>, TestType>);
}

using value_seq_t = test_seq::value_seq_t;

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence find",
    "[type traits][type sequence]",
    ((typename T, auto Expect), T, Expect),
    (int, 0) //,
    // (float, 1),
    // (void, test_seq::size())
)
{
    STATIC_REQUIRE(
        value_sequence_algo::find<value_seq_t>(basic_type_constant<T>{}, equal_to{}) == Expect
    );
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence count",
    "[type traits][type sequence]",
    ((typename T, auto Expect), T, Expect),
    (int, 1),
    (float, 2),
    (void, 0)
)
{
    STATIC_REQUIRE(
        value_sequence_algo::count<value_seq_t>(type_constant<T>{}, equal_to{}) == Expect
    );
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence append",
    "[type traits][type sequence]",
    ( //

        (auto V,typename Expect, typename FrontExpect, typename... T),
        V,
        Expect,
        FrontExpect,
        T...
    ),
    ( //
        0,
        regular_type_sequence<int, float, char, unsigned, float, void, double>,
        (regular_type_sequence<void, double, int, float, char, unsigned, float>),
        void, double
    ),
    ( //
        0,
        regular_type_sequence<int, float, char, unsigned, float, long, void*>,
        (regular_type_sequence<long, void*, int, float, char, unsigned, float>),
        long, void*
    )
)
{
    STATIC_REQUIRE(same_as<test_seq::append_t<T...>, Expect>);
    STATIC_REQUIRE(same_as<test_seq::append_front_t<T...>, FrontExpect>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence insert",
    "[type traits][type sequence]",
    ((auto Index, typename Expect, typename... T), Index, Expect, T...),
    ( //
        3,
        regular_type_sequence<int, float, char, void, double, unsigned, float>,
        void, double
    ),
    ( //
        5,
        regular_type_sequence<int, float, char, unsigned, float, long, void*>,
        long, void*
    )
)
{
    STATIC_REQUIRE(same_as<test_seq::insert_t<Index, T...>, Expect>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence remove at",
    "[type traits][type sequence]",
    ((typename Expect, auto I), Expect, I),
    (regular_type_sequence<int, char, unsigned, float>, 1),
    (regular_type_sequence<int, float, unsigned, float>, 2)
)
{
    STATIC_REQUIRE(same_as<test_seq::remove_at_t<I>, Expect>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: unique type sequence",
    "[type traits][type sequence]",
    ((auto V, typename Seq, typename Expect), V, Seq, Expect),
    (0, type_sequence<>, regular_type_sequence<>),
    (0, test_seq, regular_type_sequence<int, float, char, unsigned>)
)
{
    STATIC_REQUIRE(same_as<type_sequence_algo::unique_t<Seq, equal_to<>>, Expect>);
}
