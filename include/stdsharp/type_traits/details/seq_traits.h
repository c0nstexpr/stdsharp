#pragma once

#include "../regular_value_sequence.h"

#include <utility>

namespace stdsharp
{
    template<typename...>
    struct type_sequence;
}

namespace stdsharp::details
{
    template<typename Seq, std::size_t Size>
    struct seq_traits
    {
        template<auto... I, auto... J>
        static consteval auto
            split(std::index_sequence<I...> /*unused*/, std::index_sequence<J...> /*unused*/)
        {
            if constexpr(requires { requires std::same_as<template_rebind<Seq>, type_sequence<>>; })
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

        template<std::size_t Index>
        struct insert_trait
        {
            static constexpr auto impl = []<auto... I, auto... J>(
                                             std::index_sequence<I...> /*unused*/,
                                             std::index_sequence<J...> /*unused*/
                                         ) consteval {
                return split(std::index_sequence<I...>{}, std::index_sequence<(J + Index)...>{});
            }(std::make_index_sequence<Index>{}, std::make_index_sequence<Size - Index>{});

            template<typename U>
            using type = decltype(impl(U{}));
        };

        template<std::size_t Index>
        struct replace_trait
        {
            static constexpr auto impl = []<auto... I, auto... J>(
                                             std::index_sequence<I...> /*unused*/,
                                             std::index_sequence<J...> /*unused*/
                                         ) consteval {
                return split(
                    std::index_sequence<I...>{},
                    std::index_sequence<(J + Index + 1)...>{}
                );
            }(std::make_index_sequence<Index>{}, std::make_index_sequence<Size - Index - 1>{});

            template<typename U>
            using type = decltype(impl(U{}));
        };
    };
}