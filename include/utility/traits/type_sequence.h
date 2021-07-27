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
    using to_regular_type_sequence_t = typename take_type_sequence<Sequence>::as_sequence_t;

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
                        return std::same_as<typename seq::template get_t<I>, T>;
                    } 
                ) == I; // clang-format on
            }

            static constexpr auto filtered_indices = []<size_t... I>(const index_sequence<I...>)
            {
                array<size_t, seq::size()> indices{};
                size_t valid_size = 0;
                const auto f = [&]<size_t J>(const constant<J>) noexcept
                {
                    if(is_valid<J>())
                    {
                        indices[valid_size] = J;
                        ++valid_size;
                    }
                };

                (f(constant<I>{}), ...);

                return pair{indices, valid_size};
            }
            (typename seq::index_seq{});

            template<std::size_t... I>
            using filtered_seq = 
                regular_type_sequence<typename seq::template get_t<filtered_indices.first[I]>...>;

            using type = typename take_value_sequence< //
                make_value_sequence_t<std::size_t{}, filtered_indices.second> // clang-format off
            >::template apply_t<filtered_seq>; // clang-format on
        };

        template<typename Proj, typename Func, typename... Types>
        concept type_sequence_invocable =
            ((std::invocable<Proj, type_identity<Types>> &&
              std::invocable<Func, invoke_result_t<Proj, type_identity<Types>>>)&&...);

        template<typename Proj, 
            typename Func, typename... Types>
        concept type_sequence_nothrow_invocable =
            ((nothrow_invocable<Func, invoke_result_t<Proj, type_identity<Types>>>)&&...);

        template<typename Proj, typename Func, typename... Types>
        concept type_sequence_predicate =
            ((predicate<Func, invoke_result_t<Proj, type_identity<Types>>>)&&...);

        template<typename Proj, typename Func, typename... Types>
        concept type_sequence_nothrow_predicate =
            ((nothrow_invocable_r<Func, bool, invoke_result_t<Proj, type_identity<Types>>>)&&...);

        template<typename>
        struct from_regular_value_sequence;

        template<auto... Values>
        struct from_regular_value_sequence<regular_value_sequence<Values...>> :
            std::type_identity<type_sequence<typename decltype(Values)::type...>>
        {
        };

        template<typename Seq>
        using from_regular_value_sequence_t = typename from_regular_value_sequence<Seq>::type;
    }

    template<typename... Types>
    using reverse_type_sequence_t = typename details::reverse_type_sequence_t<Types...>::type;

    template<typename... Types>
    using unique_type_sequence_t = typename details::unique_type_sequence<Types...>::type;

    template<typename... Types>
    struct type_sequence : private traits::value_sequence<type_identity_v<Types>...>
    {
    private:
        using base = traits::value_sequence<type_identity_v<Types>...>;

        template<typename Seq>
        using from_value_seq_t = details::from_regular_value_sequence_t<Seq>;

    public:
        using typename base::index_seq;
        using base::size;
        using base::invoke;
        using base::get_range;
        using base::for_each;
        using base::for_each_n;
        using base::find_if;
        using base::find_if_not;
        using base::find;
        using base::count_if;
        using base::count_if_not;
        using base::count;
        using base::all_of;
        using base::any_of;
        using base::none_of;
        using base::contains;
        using base::adjacent_find;

        template<template<typename...> typename T>
        using apply_t = T<Types...>;

        template<std::size_t I>
        using get_t = typename decltype(base::template get<I>())::type;

        template<std::size_t I, typename... Args>
            requires std::constructible_from<get_t<I>, Args...>
        static constexpr get_t<I> get(Args&&... args) //
            noexcept(nothrow_constructible_from<get_t<I>, Args...>)
        {
            return {std::forward<Args>(args)...};
        }

        template<std::size_t... OtherInts>
        using indexed_t = regular_type_sequence<get_t<OtherInts>...>;

        template<typename Seq>
        using indexed_by_seq_t = typename take_value_sequence<Seq>::template apply_t<indexed_t>;

        template<std::size_t Size>
        using back_t = from_value_seq_t<typename base::template back_t<Size>>;

        template<std::size_t Size>
        using front_t = from_value_seq_t<typename base::template front_t<Size>>;

        template<typename... Others>
        using append_t = regular_type_sequence<Types..., Others...>;

        template<typename Seq>
        using append_by_seq_t = typename take_type_sequence<Seq>::template apply_t<append_t>;

        template<typename... Others>
        using append_front_t = regular_type_sequence<Others..., Types...>;

        template<typename Seq>
        using append_front_by_seq_t =
            typename take_type_sequence<Seq>::template apply_t<append_front_t>;

    private:
        template<std::size_t Index>
        struct insert
        {
            template<typename... Others>
            using type = typename base::template insert_t<Index, type_identity_v<Others>...>;
        };

    public:
        template<std::size_t Index, typename... Other>
        using insert_t = typename insert<Index>::template type<Other...>;

        template<std::size_t Index, typename Seq>
        using insert_by_seq_t =
            typename take_type_sequence<Seq>::template apply_t<insert<Index>::template type>;

        template<std::size_t... Index>
        using remove_at_t = from_value_seq_t<typename base::template remove_at_t<Index...>>;

        template<typename Seq>
        using remove_at_by_seq_t =
            from_value_seq_t<typename base::template remove_at_by_seq_t<Seq>>;

        template<std::size_t Index, typename Other>
        using replace_t =
            from_value_seq_t<typename base::template replace_t<Index, type_identity_v<Other>>>;

    private:
        template<std::size_t From, std::size_t... I>
        static constexpr indexed_t<From + I...> select_range_indexed( // clang-format off
            std::index_sequence<I...>
        ) noexcept; // clang-format on

    public:
        template<std::size_t From, std::size_t Size>
        using select_range_indexed_t =
            decltype(select_range_indexed<From>(std::make_index_sequence<Size>{}));
    };

    template<std::size_t I, typename... Types>
    struct std::tuple_element<I, type_sequence<Types...>> :
        std::type_identity<typename type_sequence<Types...>::template get_t<I>>
    {
    };

    template<typename... Types>
    struct std::tuple_size<type_sequence<Types...>> :
        index_constant<type_sequence<Types...>::size()>
    {
    };
}
