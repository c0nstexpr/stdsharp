#include "traits/sequence_test.h"
#include "utility/traits/sequence.h"

template<auto I, std::size_t Expect>
using get_by_index_test_params = static_params<I, Expect>;

template<template<auto...> typename T>
struct apply_t_test_params
{
};

template<typename, std::size_t...>
struct indexed_by_seq_t_test_params
{
};

template<typename Seq, typename Expect>
using seq_indexed_t_test_params = std::tuple<Seq, Expect>;

template<typename Seq, typename Expect, typename FrontExpect>
using seq_append_by_seq_t_test_params = std::tuple<Seq, Expect, FrontExpect>;

template<typename Seq, typename Expect>
using seq_remove_t_test_params = std::tuple<Seq, Expect>;

template<std::size_t, typename, typename>
struct insert_by_seq_t_test_params
{
};

boost::ut::suite& sequence_test()
{
    static boost::ut::suite suite = []
    {
        using namespace boost::ut;
        using namespace bdd;
        using namespace blurringshadow::utility;

        using test_seq = traits::sequence<0, 1, std::size_t{7}, 1, std::to_array("my literal")>;

        println(fmt::format("test_seq type: {}", reflection::type_name<test_seq>()));

        feature("construct") = []
        {
            static_expect<std::default_initializable<traits::sequence<>>>();
            static_expect<std::default_initializable<test_seq>>();
        };

        // clang-format off
        feature("get_by_index") = []<auto I, std::size_t Expect>(
            const get_by_index_test_params<I, Expect>
        ) // clang-format on
        {
            given("given index") = []
            {
                print(fmt::format("index: {}", I));

                then("indexed value should be expected value") = []
                {
                    print(fmt::format("expected value: {}", Expect));
                    static_expect<test_seq::get_by_index<I>() == _t{Expect}>();
                };
            }; // clang-format off
        } | std::tuple<
            get_by_index_test_params<0, 0>,
            get_by_index_test_params<1, 1>,
            get_by_index_test_params<2, 7>,
            get_by_index_test_params<3, 1>
        >{}; // clang-format on

        feature("invoke") = []<typename T>(const T)
        {
            given("given function") = []
            {
                print(fmt::format("function type: {}", reflection::type_name<T>()));

                // clang-format off
                then("sequence invoke should be invocable") = []
                {
                    static_expect<std::invocable<decltype(&test_seq::invoke<T>), T>>();
                }; // clang-format on
            };
        } | std::tuple<std::identity>{};

        // clang-format off
        feature("for_each") = [](auto func) { test_seq::for_each(std::move(func)); } |
            std::tuple{
                [i = 0](const auto&) mutable
                {
                    if(i == 3) return false;
                    if(i <= 2)
                    {
                        ++i;
                        return true;
                    }
                    static_expect<_b(false)>();

                    return false;
                }
            }; // clang-format on

        feature("find") = []<auto V, auto Expect>(const static_params<V, Expect>)
        {
            given("given value") = []
            {
                print(fmt::format("value: {}", V));

                then("found index should be expected") = []
                {
                    print(fmt::format("expected: {}", Expect));
                    static_expect<test_seq::find(V) == _t(Expect)>();
                };
            }; // clang-format off
        } | std::tuple<
                static_params<0, 0>,
                static_params<1, 1>,
                static_params<'!', test_seq::size()>
        >{}; // clang-format on

        feature("count") = []<auto V, auto Expect>(const static_params<V, Expect>)
        {
            given("given value") = []
            {
                print(fmt::format("value: {}", V));

                then("count should be expected") = []
                {
                    print(fmt::format("expected: {}", Expect));
                    static_expect<test_seq::count(V) == _t(Expect)>();
                };
            }; // clang-format off
        } | std::tuple<static_params<0, 1>, static_params<1, 2>, static_params<'?', 0>>{};
        // clang-format on

        feature("apply_t") = []<template<auto...> typename T>(const apply_t_test_params<T>)
        {
            given("given template") = []
            {
                // clang-format off
                print(fmt::format("template: {}", reflection::type_name<T<>>()));

                then("type that is applied sequence values should be constructible") = []
                {
                    static_expect<_b(std::default_initializable<test_seq::apply_t<T>>)>();
                };
            };
        } | std::tuple<apply_t_test_params<static_params>>{}; // clang-format on

        // TODO here the braces to avoid strange MSVC compile error
        {
            feature("transform_t") = []<auto... Functor>(const static_params<Functor...>)
            {
                static_expect<std::default_initializable<test_seq::transform_t<Functor...>>>();
                // clang-format off
            } | std::tuple<
                static_params<identity_v>,
                static_params<
                    [](const int v) mutable { return v + 1; },
                    [](const int v) { return v + 42; },
                    [](const std::size_t v) { return v + 42; },
                    [](const int v) { return v + 6; },
                    []<auto Size>(const std::array<char, Size>& str) { return str[0]; }
                >
            >{}; // clang-format on
        }

        // clang-format off
        feature("indexed_by_seq_t") = []<typename T, typename Expect>(
            const seq_indexed_t_test_params<T, Expect>
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
                    static_expect<_b(std::same_as<actual_t, Expect>)>() << // clang-format off
                        fmt::format("actual type: {}", reflection::type_name<actual_t>());
                };
            };
        } | std::tuple<
            seq_indexed_t_test_params<traits::sequence<1, 2>, traits::sequence<1, std::size_t{7}>>,
            seq_indexed_t_test_params<
                traits::sequence<2, 4>,
                traits::sequence<std::size_t{7}, std::to_array("my literal")>
            >
        >{}; // clang-format on

        // clang-format off
        feature("append_by_seq_t") = []<typename Seq, typename Expect, typename FrontExpect>(
            const seq_append_by_seq_t_test_params<Seq, Expect, FrontExpect>
        ) // clang-format on
        {
            given("given sequence") = []
            {
                print(fmt::format("sequence type: {}", reflection::type_name<Seq>()));

                then("use seq type as append_by_seq_t template arg, type should be expected") = []
                {
                    print(fmt::format("expected type: {}", reflection::type_name<Expect>()));

                    using actual_t = test_seq::append_by_seq_t<Seq>;
                    static_expect<std::same_as<actual_t, Expect>>() << // clang-format off
                        fmt::format("actual type: {}", reflection::type_name<actual_t>());
                }; // clang-format on

                then("use seq type as append_front_by_seq_t template arg, "
                     "type should be expected") = []
                {
                    print(fmt::format("expected type: {}", reflection::type_name<FrontExpect>()));
                    using actual_t = test_seq::append_front_by_seq_t<Seq>;
                    static_expect<std::same_as<actual_t, FrontExpect>>() << // clang-format off
                        fmt::format("actual type: {}", reflection::type_name<actual_t>());
                };
            };
        } | std::tuple<
            seq_append_by_seq_t_test_params<
                traits::sequence<1, 2>,
                traits::sequence<0, 1, std::size_t{7}, 1, std::to_array("my literal"), 1, 2>,
                traits::sequence<1, 2, 0, 1, std::size_t{7}, 1, std::to_array("my literal")>
            >,
            seq_append_by_seq_t_test_params<
                traits::sequence<2, 4>,
                traits::sequence<0, 1, std::size_t{7}, 1, std::to_array("my literal"), 2, 4>,
                traits::sequence<2, 4, 0, 1, std::size_t{7}, 1, std::to_array("my literal")>
            >
        >{}; // clang-format on

        // clang-format off
        feature("insert_by_seq_t") = []<std::size_t Index, typename Seq, typename Expect>(
            const insert_by_seq_t_test_params<Index, Seq, Expect>
        ) // clang-format on
        {
            given("given sequence") = []
            {
                print(fmt::format("sequence type: {}", reflection::type_name<Seq>()));

                then("use seq type as insert_by_seq_t template arg, type should be expected") = []
                {
                    print(fmt::format("expected type: {}", reflection::type_name<Expect>()));
                    using actual_t = test_seq::insert_by_seq_t<Index, Seq>;
                    static_expect<std::same_as<actual_t, Expect>>() << // clang-format off
                        fmt::format("actual type: {}", reflection::type_name<actual_t>());
                };
            }; // clang-format off
        } | std::tuple<
            insert_by_seq_t_test_params<
                3,
                traits::sequence<1, 2>,
                traits::sequence<0, 1, std::size_t{7}, 1, 2, 1, std::to_array("my literal")>
            >,
            insert_by_seq_t_test_params<
                5,
                traits::sequence<2, 4>,
                traits::sequence<0, 1, std::size_t{7}, 1, std::to_array("my literal"), 2, 4>
            >
        >{}; // clang-format on

        // clang-format off
        feature("remove_at_by_seq_t") = []<typename T, typename Expect>(
            const seq_indexed_t_test_params<T, Expect>
        ) // clang-format on
        {
            given("given indices sequence") = []
            {
                print(fmt::format("indices type: {}", reflection::type_name<T>()));

                then("use indices type as remove_at_by_seq_t template arg, "
                     "type should be expected") = []
                {
                    print(fmt::format("expected type: {}", reflection::type_name<Expect>()));
                    static_expect<_b(std::same_as<test_seq::remove_at_by_seq_t<T>, Expect>)>();
                };
            }; // clang-format off
        } | std::tuple<
            seq_indexed_t_test_params<
                traits::sequence<1, 2>,
                traits::sequence<0, 1, std::to_array("my literal")>
            >,
            seq_indexed_t_test_params<traits::sequence<2, 4>, traits::sequence<0, 1, 1>>
        >{}; // clang-format on
    };

    return suite;
}
