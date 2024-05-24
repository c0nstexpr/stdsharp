#pragma once

#include "../regular_value_sequence.h"

#include <utility>

namespace stdsharp
{
    template<auto...>
    struct value_sequence;

    template<typename...>
    struct type_sequence;
}

namespace stdsharp::details
{
    template<typename Seq>
    concept is_type_sequence =
        requires { requires std::same_as<template_rebind<Seq>, type_sequence<>>; };

    template<typename Seq, std::size_t Size>
    struct seq_traits
    {
        template<auto... I, auto... J>
        static consteval auto
            split(std::index_sequence<I...> /*unused*/, std::index_sequence<J...> /*unused*/)
        {
            if constexpr(is_type_sequence<Seq>)
                return []<typename... T>(basic_type_sequence<T...> /*unused*/) consteval
                {
                    return regular_type_sequence<
                        typename Seq::template type<I>...,
                        T...,
                        typename Seq::template type<J>...>{};
                };
            else
                return []<auto... V>(regular_value_sequence<V...> /*unused*/) consteval {
                    return regular_value_sequence<
                        Seq::template get<I>()...,
                        V...,
                        Seq::template get<J>()...>{};
                };
        };

        template<std::size_t Index, std::size_t Offset = 0>
        struct insert_trait
        {
            static constexpr auto impl =
                []<auto... I>(auto tag, std::index_sequence<I...> /*unused*/) consteval
            {
                return split(tag, std::index_sequence<(I + Index + Offset)...>{});
            }(std::make_index_sequence<Index>{}, std::make_index_sequence<Size - Index - Offset>{});

            template<typename U>
            using type = decltype(impl(U{}));
        };

        template<std::size_t Index>
        using replace_trait = insert_trait<Index, 1>;
    };

    template<std::size_t... I>
    static consteval auto seq_apply(std::index_sequence<I...> /*unused*/)
    {
        if constexpr(is_type_sequence<Seq>)
            return regular_type_sequence<typename Seq::template type<I...>>{};
        else return regular_value_sequence<Seq::template get<I>()...>{};
    };

    template<typename Seq>
    struct reverse_sequence_traits
    {
        using seq = Seq;

        static constexpr auto size = Seq::size();

        template<std::size_t... I>
        static consteval std::index_sequence<(size - I - 1)...> impl(std::index_sequence<I...>);


        using type = decltype(Apply(impl(std::make_index_sequence<size>{})));
    };


    template<typename, typename>
    struct unique_type_sequence;

    template<typename... T, std::constructible_from Comp>
    struct unique_type_sequence<type_sequence<T...>, Comp>
    {
        using seq = type_sequence<T...>;

        static consteval auto size() noexcept { return seq::size(); }

        template<std::size_t I>
        static consteval auto fill_indices(auto& indices, std::size_t& i, Comp& comp)
        {
            using value_seq = seq::value_seq_t;

            const auto found =
                stdsharp::value_sequence_algo::find<value_seq>(value_seq::template get<I>(), comp);
            if(found < i) return;
            indices[i++] = I;
        }

        template<auto... I>
        static consteval auto get_indices(const std::index_sequence<I...> /*unused*/)
        {
            std::array<std::size_t, size()> indices{};
            std::size_t i = 0;
            Comp comp{};
            (fill_indices<I>(indices, i, comp), ...);
            return std::pair{indices, i};
        }

        static constexpr auto indices_pair = get_indices(std::make_index_sequence<size()>{});

        static constexpr auto indices = indices_pair.first;

        static constexpr auto indices_size = indices_pair.second;

        template<auto... I>
        static consteval regular_type_sequence<typename seq::template type<indices[I]>...>
            apply_indices(std::index_sequence<I...>);

        using type = decltype(apply_indices(std::make_index_sequence<indices_size>{}));
    };
}