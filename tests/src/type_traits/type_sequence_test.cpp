#include "type_traits/type_sequence_test.h"
#include "type_traits/type_sequence.h"

namespace stdsharp::test::type_traits
{
    namespace
    {
        template<auto, typename>
        struct get_test_params
        {
        };

        template<typename, size_t>
        struct find_test_params
        {
        };

        template<typename T, size_t Expect>
        using count_test_params = find_test_params<T, Expect>;

        template<template<typename...> typename T>
        struct apply_t_test_params
        {
        };

        template<typename, typename...>
        struct indexed_by_seq_t_test_params
        {
        };

        template<typename Seq, typename Expect>
        using indexed_t_test_params = stdsharp::type_traits::regular_type_sequence<Seq, Expect>;

        template<typename Seq, typename Expect, typename FrontExpect>
        using append_by_seq_t_test_params =
            stdsharp::type_traits::regular_type_sequence<Seq, Expect, FrontExpect>;

        template<typename Seq, typename Expect>
        using remove_t_test_params = stdsharp::type_traits::regular_type_sequence<Seq, Expect>;

        template<size_t, typename, typename>
        struct insert_by_seq_t_test_params
        {
        };

        template<typename, typename...>
        struct unique_seq_t_test_params
        {
        };
    }

    template<typename... T>
    using type_sequence = stdsharp::type_traits::type_sequence<T...>;

    template<auto... V>
    using regular_value_sequence = stdsharp::type_traits::regular_value_sequence<V...>;

    template<typename... T>
    using regular_type_sequence = stdsharp::type_traits::regular_type_sequence<T...>;

    boost::ut::suite& type_sequence_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace std;
            using namespace literals;
            using namespace boost::ut;
            using namespace bdd;
            using namespace type_traits;

            using test_seq = type_sequence<int, float, char, unsigned, float>;

            println(fmt::format("test_seq type: {}", reflection::type_name<test_seq>()));

            feature("construct") = []
            {
                static_expect<default_initializable<type_sequence<>>>();
                static_expect<default_initializable<test_seq>>();
            };

            feature("get_t") = []<auto I, typename Expect>(const get_test_params<I, Expect>)
            {
                given("given index") = []
                {
                    print(fmt::format("index: {}", I));

                    then("indexed value should be expected type") = []
                    {
                        using actual_t = test_seq::get_t<I>;

                        print(fmt::format("expected type: {}", reflection::type_name<Expect>()));
                        static_expect<same_as<actual_t, Expect>>() << //
                            fmt::format("actual type: {}", reflection::type_name<actual_t>());
                    };
                }; // clang-format off
            } | tuple<
                get_test_params<0, int>,
                get_test_params<1, float>,
                get_test_params<2, char>,
                get_test_params<3, unsigned>
            >{}; // clang-format on

            feature("invoke") = []<typename T>(const T)
            {
                given("given function") = []
                {
                    print(fmt::format("function type: {}", reflection::type_name<T>()));

                    then("sequence invoke should be invocable") = [] //
                    {
                        static_expect<invocable<decltype(test_seq::invoke), T>>(); //
                    };
                };
            } | tuple<identity>{};

            feature("find") = []<typename T, auto Expect>(const find_test_params<T, Expect>)
            {
                given("given type") = []
                {
                    print(fmt::format("type: {}", reflection::type_name<T>()));

                    then("found index should be expected") = []
                    {
                        constexpr auto i =
                            test_seq::find(stdsharp::type_traits::type_constant_v<T>);
                        print(fmt::format("expected: {}", Expect));
                        static_expect<i == Expect>() << fmt::format("actual index: {}", i);
                    };
                }; // clang-format off
            } | tuple<
                find_test_params<float, 1>,
                find_test_params<char, 2>,
                find_test_params<void, test_seq::size>
            >{}; // clang-format on

            feature("count") = []<typename T, auto Expect>(const count_test_params<T, Expect>)
            {
                given("given type") = []
                {
                    print(fmt::format("type: {}", reflection::type_name<T>()));

                    then("count should be expected") = []
                    {
                        constexpr auto count =
                            test_seq::count(stdsharp::type_traits::type_constant_v<T>);
                        print(fmt::format("expected: {}", Expect));
                        static_expect<count == Expect>() << fmt::format("actual count: {}", count);
                    };
                }; // clang-format off
            } | tuple<
                count_test_params<int, 1>,
                count_test_params<float, 2>,
                count_test_params<void, 0>
            >{};

            // clang-format on
            feature("apply_t") = []<template<typename...> typename T>(const apply_t_test_params<T>)
            {
                given("given template") = []
                {
                    print(fmt::format("template: {}", reflection::type_name<T<>>()));

                    then("type that is applied sequence types should be constructible") = []
                    {
                        static_expect<_b(default_initializable<test_seq::apply_t<T>>)>(); //
                    };
                }; // clang-format off
            } | tuple<apply_t_test_params<regular_type_sequence>>{}; // clang-format on

            // clang-format off
            feature("indexed_by_seq_t") = []<typename T, typename Expect>(
                const indexed_t_test_params<T, Expect>
            ) // clang-format on
            {
                given("given indices sequence") = []
                {
                    print(fmt::format("indices type: {}", reflection::type_name<T>()));

                    then("use indices type as indexed_by_seq_t template arg, "
                         "type should be expected") = []
                    {
                        print(fmt::format("expected type: {}", reflection::type_name<Expect>()));
                        using actual_t = test_seq::indexed_by_seq_t<T>;
                        static_expect<_b(same_as<actual_t, Expect>)>() << //
                            fmt::format("actual type: {}", reflection::type_name<actual_t>());
                    };
                }; // clang-format off
            } | tuple<
                indexed_t_test_params<
                    regular_value_sequence<1, 2>,
                    regular_type_sequence<float, char>
                >,
                indexed_t_test_params<
                    regular_value_sequence<2, 4>,
                    regular_type_sequence<char, float>
                >
            >{}; // clang-format on

            // clang-format off
            feature("append_by_seq_t") = []<typename Seq, typename Expect, typename FrontExpect>(
                const append_by_seq_t_test_params<Seq, Expect, FrontExpect>
            ) // clang-format on
            {
                given("given sequence") = []
                {
                    print(fmt::format("sequence type: {}", reflection::type_name<Seq>()));

                    then("use seq type as append_by_seq_t template arg, type should be expected") =
                        []
                    {
                        print(fmt::format("expected type: {}", reflection::type_name<Expect>()));

                        using actual_t = test_seq::append_by_seq_t<Seq>;
                        static_expect<same_as<actual_t, Expect>>() << //
                            fmt::format("actual type: {}", reflection::type_name<actual_t>());
                    };

                    then("use seq type as append_front_by_seq_t template arg, "
                         "type should be expected") = []
                    {
                        print( // clang-format off
                            fmt::format("expected type: {}", reflection::type_name<FrontExpect>()) 
                        ); // clang-format on

                        using actual_t = test_seq::append_front_by_seq_t<Seq>;
                        static_expect<same_as<actual_t, FrontExpect>>() << //
                            fmt::format("actual type: {}", reflection::type_name<actual_t>());
                    };
                }; // clang-format off
            } | tuple<
                append_by_seq_t_test_params<
                    regular_type_sequence<void, int*>,
                    regular_type_sequence<int, float, char, unsigned, float, void, int*>,
                    regular_type_sequence<void, int*, int, float, char, unsigned, float>
                >
            >{}; // clang-format on

            // clang-format off
            feature("insert_by_seq_t") = []<size_t Index, typename Seq, typename Expect>(
                const insert_by_seq_t_test_params<Index, Seq, Expect>
            ) // clang-format on
            {
                given("given sequence") = []
                {
                    print(fmt::format("sequence type: {}", reflection::type_name<Seq>()));

                    then("use seq type as insert_by_seq_t template arg, type should be expected") =
                        []
                    {
                        print(fmt::format("expected type: {}", reflection::type_name<Expect>()));
                        using actual_t = test_seq::insert_by_seq_t<Index, Seq>;
                        static_expect<same_as<actual_t, Expect>>() << //
                            fmt::format("actual type: {}", reflection::type_name<actual_t>());
                    };
                }; // clang-format off
            } | tuple<
                insert_by_seq_t_test_params<
                    3,
                    regular_type_sequence<void, int*>,
                    regular_type_sequence<int, float, char, void, int*, unsigned, float>
                >
            >{}; // clang-format on

            // clang-format off
            feature("remove_at_by_seq_t") = []<typename T, typename Expect>(
                const indexed_t_test_params<T, Expect>
            ) // clang-format on
            {
                given("given indices sequence") = []
                {
                    print(fmt::format("indices type: {}", reflection::type_name<T>()));

                    then("use indices type as remove_at_by_seq_t template arg, "
                         "type should be expected") = []
                    {
                        print(fmt::format("expected type: {}", reflection::type_name<Expect>()));
                        using actual_t = test_seq::remove_at_by_seq_t<T>;
                        static_expect<_b(same_as<actual_t, Expect>)>() << //
                            fmt::format("actual type: {}", reflection::type_name<actual_t>());
                    };
                }; // clang-format off
            } | tuple<
                indexed_t_test_params<
                    regular_value_sequence<1, 2>,
                    regular_type_sequence<int, unsigned, float>
                >
            >{}; // clang-format on

            // clang-format off
            feature("unique_type_sequence_t") = []<typename Expect, typename... Types>(
                const unique_seq_t_test_params<Expect, Types...>
            ) // clang-format on
            {
                given("given types") = []
                {
                    print( //
                        fmt::format(
                            "types: {}", // clang-format off
                            fmt::join(tuple{reflection::type_name<Types>()...}, ", ")
                        ) // clang-format on
                    );

                    then("use seq as unique_type_sequence_t template arg, "
                         "type should be expected") = []
                    {
                        print(fmt::format("expected type: {}", reflection::type_name<Expect>()));
                        using actual_t = stdsharp::type_traits::unique_type_sequence_t<Types...>;
                        static_expect<_b(same_as<actual_t, Expect>)>() << //
                            fmt::format("actual type: {}", reflection::type_name<actual_t>());
                    };
                }; // clang-format off
            } | tuple<
                unique_seq_t_test_params<regular_type_sequence<>>,
                unique_seq_t_test_params<
                    regular_type_sequence<int, float, void, char>,
                    int, float, int, void, char, void
                >
            >{}; // clang-format on
        };

        return suite;
    }
}
