#include "type_traits/value_sequence_test.h"
#include "type_traits/value_sequence.h"

namespace stdsharp::test::type_traits
{
    namespace
    {
        template<auto I, size_t Expect>
        using get_test_params = stdsharp::type_traits::regular_value_sequence<I, Expect>;

        template<template<auto...> typename T>
        struct apply_t_test_params
        {
        };

        template<typename Seq, typename Expect>
        using remove_t_test_params = std::tuple<Seq, Expect>;

        template<typename, auto...>
        struct unique_seq_t_test_params
        {
        };
    }

    template<auto... V>
    using value_sequence = stdsharp::type_traits::value_sequence<V...>;

    template<auto... V>
    using regular_value_sequence = stdsharp::type_traits::regular_value_sequence<V...>;

    boost::ut::suite& value_sequence_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace std;
            using namespace literals;
            using namespace boost::ut;
            using namespace bdd;
            using namespace functional;
            using namespace utility;

            using test_seq = value_sequence<0, 1, size_t{7}, 1, to_array("my literal")>;

            println(fmt::format("test_seq type: {}", reflection::type_name<test_seq>()));

            construct_feat<value_sequence<>, test_seq>();

            feature("get") = []<auto I, size_t Expect>(const get_test_params<I, Expect>)
            {
                given("given index") = []
                {
                    print(fmt::format("index: {}", I));

                    then("indexed value should be expected value") = []
                    {
                        print(fmt::format("expected value: {}", Expect));
                        static_expect<test_seq::get<I>() == Expect>();
                    };
                }; // clang-format off
            } | tuple<
                get_test_params<0, 0>,
                get_test_params<1, 1>,
                get_test_params<2, 7>,
                get_test_params<3, 1>
            >{}; // clang-format on

            feature("invoke") = invoke_feat<test_seq>() | tuple<identity>{};

            feature("find") = []<auto V, auto Expect>(const regular_value_sequence<V, Expect>)
            {
                given("given value") = []
                {
                    print(fmt::format("value: {}", V));

                    then("found index should be expected") = []
                    {
                        constexpr auto i = test_seq::find(V);
                        print(fmt::format("expected: {}", Expect));
                        static_expect<i == Expect>() << fmt::format("actual value: {}", i);
                    };
                }; // clang-format off
            } | tuple<
                regular_value_sequence<0, 0>,
                regular_value_sequence<1, 1>,
                regular_value_sequence<'!', test_seq::size>
            >{}; // clang-format on

            feature("count") = []<auto V, auto Expect>(const regular_value_sequence<V, Expect>)
            {
                given("given value") = []
                {
                    print(fmt::format("value: {}", V));

                    then("count should be expected") = []
                    {
                        constexpr auto count = test_seq::count(V);
                        print(fmt::format("expected: {}", Expect));
                        static_expect<count == Expect>() << fmt::format("actual count: {}", count);
                    };
                }; // clang-format off
            } | tuple<regular_value_sequence<0, 1>, regular_value_sequence<1, 2>, regular_value_sequence<'?', 0>>{};

            // clang-format on
            feature("apply_t") = []<template<auto...> typename T>(const apply_t_test_params<T>)
            {
                given("given template") = []
                {
                    print(fmt::format("template: {}", reflection::type_name<T<>>()));

                    then("type that is applied sequence values should be constructible") = []
                    {
                        static_expect<_b(default_initializable<test_seq::apply_t<T>>)>(); //
                    };
                }; // clang-format off
            } | tuple<apply_t_test_params<regular_value_sequence>>{}; // clang-format on

            // TODO clang lambda NTTP
            constexpr auto lam_1 = [](const int v) mutable { return v + 1; };
            constexpr auto lam_2 = [](const int v) { return v + 42; };
            constexpr auto lam_3 = [](const size_t v) { return v + 42; };
            constexpr auto lam_4 = [](const int v) { return v + 6; };
            constexpr auto lam_5 = []<auto Size>(const array<char, Size>& str) { return str[0]; };

            feature("transform_t") = []<auto... Functor>(const regular_value_sequence<Functor...>)
            {
                static_expect<default_initializable<test_seq::template transform_t<Functor...>>>();
                // clang-format off
            } | tuple<
                regular_value_sequence<identity_v>,
                regular_value_sequence<lam_1, lam_2, lam_3, lam_4, lam_5>
            >{}; // clang-format on

            // clang-format off
            feature("indexed_by_seq_t") = indexed_by_seq_t_feat<test_seq>() | tuple<
                indexed_by_seq_t_test_params<
                    regular_value_sequence<1, 2>,
                    regular_value_sequence<1, size_t{7}>
                >,
                indexed_by_seq_t_test_params<
                    regular_value_sequence<2, 4>,
                    regular_value_sequence<size_t{7}, to_array("my literal")>
                >
            >{}; // clang-format on

            // clang-format off
            feature("append_by_seq_t") = append_by_seq_t_feat<test_seq>() | tuple<
                append_by_seq_t_test_params<
                    regular_value_sequence<1, 2>,
                    regular_value_sequence<
                        0, 1, size_t{7}, 1, to_array("my literal"), 1, 2
                    >,
                    regular_value_sequence<
                        1, 2, 0, 1, size_t{7}, 1, to_array("my literal")
                    >
                >,
                append_by_seq_t_test_params<
                    regular_value_sequence<2, 4>,
                    regular_value_sequence<
                        0, 1, size_t{7}, 1, to_array("my literal"), 2, 4
                    >,
                    regular_value_sequence<
                        2, 4, 0, 1, size_t{7}, 1, to_array("my literal")
                    >
                >
            >{}; // clang-format on

            // clang-format off
            feature("insert_by_seq_t") = insert_by_seq_feat<test_seq>() | tuple<
                insert_by_seq_t_test_params<
                    3,
                    regular_value_sequence<1, 2>,
                    regular_value_sequence<
                        0, 1, size_t{7}, 1, 2, 1, to_array("my literal")
                    >
                >,
                insert_by_seq_t_test_params<
                    5,
                    regular_value_sequence<2, 4>,
                    regular_value_sequence<
                        0, 1, size_t{7}, 1, to_array("my literal"), 2, 4
                    >
                >
            >{}; // clang-format on

            // clang-format off
            feature("remove_at_by_seq_t") = remove_at_by_seq_t_feat<test_seq>() | tuple<
                indexed_by_seq_t_test_params<
                    regular_value_sequence<1, 2>,
                    regular_value_sequence<0, 1, to_array("my literal")>
                >,
                indexed_by_seq_t_test_params<
                    regular_value_sequence<2, 4>,
                    regular_value_sequence<0, 1, 1>
                >
            >{}; // clang-format on

            // clang-format off
            feature("unique_value_sequence_t") = []<typename Expect, auto... Values>(
                const unique_seq_t_test_params<Expect, Values...>
            ) // clang-format on
            {
                given("given values") = []
                {
                    print( //
                        fmt::format(
                            "values: {}", // clang-format off
                            fmt::join(std::tuple{Values...}, ", ")
                        ) // clang-format on
                    );

                    then("use seq as unique_value_sequence_t template arg, "
                         "type should be expected") = []
                    {
                        print(fmt::format("expected type: {}", reflection::type_name<Expect>()));
                        using actual_t = stdsharp::type_traits::unique_value_sequence_t<Values...>;
                        static_expect<_b(same_as<actual_t, Expect>)>() << //
                            fmt::format("actual type: {}", reflection::type_name<actual_t>());
                    };
                }; // clang-format off
            } | tuple<
                unique_seq_t_test_params<regular_value_sequence<>>,
                unique_seq_t_test_params<
                    regular_value_sequence<0, 10, 1, 5>,
                    0, 10, 1, 5, 10, 1
                >
            >{}; // clang-format on
        };

        return suite;
    }
}
