
#pragma once

#include "value_sequence.h"
#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename>
    struct type_seq_converter;

    template<template<auto...> typename Seq, auto... V>
    struct type_seq_converter<Seq<V...>>
    {
        using type = regular_type_sequence<::meta::_t<decltype(V)>...>;
    };

    template<typename Seq>
    using type_seq_converter_t = type_seq_converter<Seq>::type;

    template<typename Base, typename... Types>
    struct STDSHARP_EBO type_sequence : private Base, regular_type_sequence<Types...>
    {
        using Base::adjacent_find;
        using Base::all_of;
        using Base::any_of;
        using Base::contains;
        using Base::count;
        using Base::count_if;
        using Base::count_if_not;
        using Base::find;
        using Base::find_if;
        using Base::find_if_not;
        using Base::for_each;
        using Base::for_each_n;
        using Base::none_of;
        using Base::size;

        template<template<typename...> typename T>
        using apply_t = T<Types...>;

        template<std::size_t I>
        using type = Base::template value_type<I>::type;

        template<std::size_t... I>
        using at_t = regular_type_sequence<type<I>...>;

        template<std::size_t Size>
        using back_t = type_seq_converter_t<typename Base::template back_t<Size>>;

        template<std::size_t Size>
        using front_t = type_seq_converter_t<typename Base::template front_t<Size>>;

        template<typename... Others>
        using append_t = regular_type_sequence<Types..., Others...>;

        template<typename T = void>
        using invoke_fn = Base::template invoke_fn<T>;

        template<typename T = empty_t>
        static constexpr invoke_fn<T> invoke{};

        template<typename... Others>
        using append_front_t = regular_type_sequence<Others..., Types...>;

        template<std::size_t Index, typename... Other>
        using insert_t = type_seq_converter_t< //
            typename Base::template insert_t<
                Index,
                basic_type_constant<Other>{}... // clang-format off
                >
            >; // clang-format on

        template<std::size_t... Index>
        using remove_at_t = type_seq_converter_t<typename Base::template remove_at_t<Index...>>;

        template<std::size_t Index, typename Other>
        using replace_t = type_seq_converter_t<
            typename Base::template replace_t<Index, basic_type_constant<Other>{}>>;

        template<std::size_t From, std::size_t Size>
        using select_range_t =
            type_seq_converter_t<typename Base::template select_range_t<From, Size>>;
    };
}

namespace stdsharp
{
    template<typename... Types>
    using type_sequence = adl_proof_t<
        details::type_sequence,
        value_sequence<basic_type_constant<Types>{}...>,
        Types...>;

    template<typename T>
    using to_type_sequence = decltype( //
        []<template<typename...> typename Inner, typename... U>(const Inner<U...>&) //
        {
            return type_sequence<U...>{}; //
        }(std::declval<T>())
    );

    template<typename... T>
    using reverse_type_sequence =
        details::type_seq_converter_t<reverse_value_sequence<std::type_identity<T>{}...>>;

    template<typename... T>
    using unique_type_sequence =
        details::type_seq_converter_t<unique_value_sequence<basic_type_constant<T>{}...>>;
}

namespace std
{
    template<::stdsharp::adl_proofed_for<stdsharp::details::type_sequence> Seq>
    struct tuple_size<Seq> : stdsharp::index_constant<Seq::size()>
    {
    };

    template<std::size_t I, ::stdsharp::adl_proofed_for<stdsharp::details::type_sequence> Seq>
    struct tuple_element<I, Seq>
    {
        using type = Seq::template type<I>;
    };
}

#include "../compilation_config_out.h"