#include "traits/type_sequence_test.h"
#include "utility/traits/type_sequence.h"

namespace blurringshadow::test::utility::traits
{
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

    template<typename, auto...>
    struct unique_seq_t_test_params
    {
    };

    template<typename... T>
    using type_sequence = blurringshadow::utility::traits::type_sequence<T...>;

    template<typename... T>
    using regular_type_sequence = blurringshadow::utility::traits::regular_type_sequence<T...>;

    boost::ut::suite& type_sequence_test()
    {
        static boost::ut::suite suite = []() noexcept
        {
            using namespace boost::ut;
            using namespace bdd;
            using namespace blurringshadow::utility;

            using test_seq = type_sequence<int, char, bool>;
        };

        return suite;
    }
}
