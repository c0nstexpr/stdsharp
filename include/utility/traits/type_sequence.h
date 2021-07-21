// Created by BlurringShadow at 2021-03-03-下午 4:35

#pragma once
#include "value_sequence.h"

namespace blurringshadow::utility::traits
{
    template<typename...>
    struct regular_type_sequence
    {
    };

    template<typename...>
    struct type_sequence;

    template<typename>
    struct take_type_sequence;

    template<template<typename...> typename T, typename... Types>
    struct take_type_sequence<T<Types...>>
    {
        template<template<typename...> typename U>
        using apply_t = U<Types...>;

        using as_sequence_t = regular_type_sequence<Types...>;

        using as_type_sequence_t = type_sequence<Types...>;
    };

    template<typename Sequence>
    using to_regular_type_sequence_t = typename take_type_sequence<Sequence>::as_vsequence_t;

    template<typename Sequence>
    using to_type_sequence_t = typename take_type_sequence<Sequence>::as_type_sequence_t;

    namespace details
    {
        template<typename... Types>
        struct reverse_type_sequence_t
        {
            using seq = type_sequence<Types...>;
            using type = typename seq::template indexed_by_seq_t<
                make_value_sequence_t<seq::size() - 1, seq::size(), minus_v> // clang-format off
            >; // clang-format on
        };

        template<typename... Types>
        struct unique_type_sequence
        {
            using seq = type_sequence<Types...>;

            template<size_t I>
            static constexpr auto is_valid()
            {
                return seq::find_if( // clang-format off
                    [j = size_t{0}]<typename T>(const type_identity<T>) mutable
                    {
                        if(j == I) return true;
                        ++j;
                        return std::same_as<seq::template get_by_index<I>, T>;
                    } 
                ) == I; // clang-format on
            }

            static constexpr auto filtered_indices = []<size_t... I>(const index_sequence<I...>)
            {
                array<size_t, seq::size()> indices{};
                size_t valid_size = 0;
                const auto f = [&]<size_t J>(const type_identity<J>) noexcept
                {
                    if(is_valid<J>())
                    {
                        indices[valid_size] = J;
                        ++valid_size;
                    }
                };

                (f(type_identity<I>{}), ...);

                return pair{indices, valid_size};
            }
            (typename seq::index_seq{});

            template<std::size_t... I>
            using filtered_seq =
                regular_value_sequence<seq::template get_by_index<filtered_indices.first[I]>()...>;

            using type = typename take_type_sequence< //
                make_value_sequence_t<std::size_t{}, filtered_indices.second> // clang-format off
            >::template apply_t<filtered_seq>; // clang-format on
        };

        template<typename Proj, typename Func, typename... Types>
        concept type_sequence_invocable =
            ((std::invocable<Proj, type_identity<Types>> &&
              std::invocable<Func, invoke_result_t<Proj, type_identity<Types>>>)&&...);

        template<typename Proj, typename Func, typename... Types>
        concept type_sequence_nothrow_invocable =
            ((nothrow_invocable<Func, invoke_result_t<Proj, type_identity<Types>>>)&&...);

        template<typename Proj, typename Func, typename... Types>
        concept type_sequence_predicate =
            ((predicate<Func, invoke_result_t<Proj, type_identity<Types>>>)&&...);

        template<typename Proj, typename Func, typename... Types>
        concept type_sequence_nothrow_predicate =
            ((nothrow_invocable_r<Func, bool, invoke_result_t<Proj, type_identity<Types>>>)&&...);
    }

    template<typename... Types>
    using reverse_type_sequence_t = typename details::reverse_type_sequence_t<Types...>::type;

    template<typename... Types>
    using unique_type_sequence_t = typename details::unique_type_sequence<Types...>::type;

    template<typename... Types>
    struct type_sequence : private traits::value_sequence<std::type_identity<Types>{}...>
    {
    private:
        using base = traits::value_sequence<std::type_identity<Types>{}...>;

    public:
        using base::index_seq;
        using base::size;

        template<typename Func>
            requires(std::invocable<Func, type_identity<Types>>&&...)
        static constexpr decltype(auto) invoke(Func&& func) //
            noexcept((nothrow_invocable<Func, type_identity<Types>> && ...))
        {
            const auto f = [&func]<typename T>(const type_identity<T> v) //
                noexcept(nothrow_invocable<Func, type_identity<T>>)
            {
                return std::invoke(std::forward<Func>(func), v); //
            };
            return merge_invoke(std::bind(f, type_identity<Types>)...);
        }

        template<template<typename...> typename T>
        using apply_t = T<Types...>;

        template<std::size_t I>
        using get_by_index_t = typename decltype(base::template get_by_index<I>())::type;

        template<std::size_t... OtherInts>
        using indexed_t = regular_type_sequence<get_by_index_t<OtherInts>...>;
    };

    template<std::size_t I, typename... Types>
    struct std::tuple_element<I, type_sequence<Types...>> :
        std::type_identity<typename type_sequence<Types...>::template get_by_index_t<I>>
    {
    };

    template<typename... Types>
    struct std::tuple_size<type_sequence<Types...>> :
        index_constant<type_sequence<Types...>::size()>
    {
    };
}
