#include "stdsharp/type_traits/type_sequence.h"
#include "test.h"

using namespace std;
using namespace fmt;
using namespace stdsharp;

using test_seq = type_sequence<int, float, char, unsigned, float>;

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: type sequence",
    "[type traits]",
    test_seq,
    type_sequence<>
)
{
    STATIC_REQUIRE(default_initializable<TestType>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence type",
    "[type traits]",
    ((auto Index, typename Expect), Index, Expect),
    (0, int),
    (1, float),
    (2, char),
    (3, unsigned)
)
{
    STATIC_REQUIRE(same_as<test_seq::type<Index>, Expect>);
}

TEMPLATE_TEST_CASE( // NOLINT
    "Scenario: type sequence invoke",
    "[type traits]",
    identity,
    decltype( //
        []( //
            const stdsharp::same_as_any<
                basic_type_constant<int>,
                basic_type_constant<char>,
                basic_type_constant<unsigned>,
                basic_type_constant<float>> auto
        ) {}
    )
)
{
    STATIC_REQUIRE(invocable<test_seq::invoke_fn<>, TestType>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence find",
    "[type traits]",
    ((typename T, auto Expect), T, Expect),
    (int, 0) //,
    // (float, 1),
    // (void, test_seq::size())
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
    (void, 0)
)
{
    STATIC_REQUIRE(test_seq::count(type_constant<T>{}) == Expect);
}

SCENARIO("type sequence apply", "[type traits]") // NOLINT
{
    STATIC_REQUIRE(default_initializable<test_seq::apply_t<type_sequence>>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence at",
    "[type traits]",
    ((typename Expect, auto... V), Expect, V...),
    (regular_type_sequence<float, char>, 1, 2),
    (regular_type_sequence<char, float>, 2, 4)
)
{
    STATIC_REQUIRE(same_as<test_seq::at_t<V...>, Expect>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: type sequence append",
    "[type traits]",
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
    "[type traits]",
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
    "[type traits]",
    ((typename Expect, auto... I), Expect, I...),
    ( //
        regular_type_sequence<int, unsigned, float>,
        1,
        2
    ),
    ( //
        regular_type_sequence<int, float, unsigned>,
        2,
        4
    )
)
{
    STATIC_REQUIRE(same_as<test_seq::remove_at_t<I...>, Expect>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: unique type sequence",
    "[type traits]",
    ((auto V, typename Seq, typename Expect), V, Seq, Expect),
    (0, type_sequence<>, regular_type_sequence<>),
    (0, test_seq, regular_type_sequence<int, float, char, unsigned>)
)
{
    STATIC_REQUIRE( //
        same_as<
            typename Seq::template apply_t<unique_type_sequence>,
            Expect // clang-format off
        > // clang-format on
    );
}
