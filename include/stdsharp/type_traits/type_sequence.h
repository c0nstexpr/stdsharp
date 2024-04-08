
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
        using type = regular_type_sequence<typename decltype(V)::type...>;
    };

    template<typename Seq>
    using type_seq_converter_t = type_seq_converter<Seq>::type;

    template<typename... Types>
    struct type_sequence
    {
    private:
        using value_sequence = value_sequence<basic_type_constant<Types>{}...>;

    public:
        static constexpr std::size_t size() noexcept { return sizeof...(Types); }

        template<std::size_t I>
        using type = type_at<I, Types...>;

        template<template<typename...> typename T>
        using apply_t = T<Types...>;

        template<typename... Others>
        using append_t = regular_type_sequence<Types..., Others...>;

        template<auto... Func>
        using transform_fn = value_sequence::template transform_fn<Func...>;

        template<auto... Func>
        static constexpr transform_fn<Func...> transform{};

        template<auto... Func>
            requires std::invocable<transform_fn<Func...>>
        using transform_t = decltype(transform<Func...>());

        template<typename T = void>
        using invoke_fn = value_sequence::template invoke_fn<T>;

        template<typename T = void>
        static constexpr invoke_fn<T> invoke{};

        using for_each_fn = value_sequence::for_each_fn;

        static constexpr for_each_fn for_each{};

        template<std::size_t From, std::size_t Size>
        using select_range_t =
            type_seq_converter_t<typename value_sequence::template select_range_t<From, Size>>;

        template<typename... Others>
        using append_front_t = regular_type_sequence<Others..., Types...>;

        template<std::size_t Index, typename... Other>
        using insert_t = type_seq_converter_t<
            typename Base::template insert_t<Index, basic_type_constant<Other>{}...>>;

        template<std::size_t... Index>
        using remove_at_t = type_seq_converter_t<typename Base::template remove_at_t<Index...>>;

        template<std::size_t Index, typename Other>
        using replace_t = type_seq_converter_t<
            typename Base::template replace_t<Index, basic_type_constant<Other>{}>>;

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
        []<template<typename...> typename Inner, typename... U>(const Inner<U...>&)
        {
            return type_sequence<U...>{}; //
        }(std::declval<T>())
    );
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