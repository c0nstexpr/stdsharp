#pragma once

#include "value_sequence.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename... T>
    struct type_sequence
    {
        using value_seq_t = value_sequence<basic_type_constant<T>{}...>;

        using regular_type_seq = regular_type_sequence<T...>;

    private:
        template<auto... Func>
        struct transform
        {
            using type = regular_type_sequence<
                typename decltype(stdsharp::invoke(Func, basic_type_constant<T>{}))::type...>;
        };

        template<auto Func>
        struct transform<Func>
        {
            using type = regular_type_sequence<
                typename decltype(stdsharp::invoke(Func, basic_type_constant<T>{}))::type...>;
        };

    public:
        static constexpr std::size_t size() noexcept { return sizeof...(T); }

        template<std::size_t I>
        using type = type_at<I, T...>;

        template<template<typename...> typename U>
        using apply_t = U<T...>;

        template<auto... Func>
        using transform_t = transform<Func...>::type;

        template<typename U = void>
        using invoke_fn = value_seq_t::template invoke_fn<U>;

        template<typename U = void>
        static constexpr invoke_fn<U> invoke{};

        using for_each_fn = value_seq_t::for_each_fn;

        static constexpr for_each_fn for_each{};

        template<typename... Others>
        using append_t = regular_type_sequence<T..., Others...>;

        template<typename... Others>
        using append_front_t = regular_type_sequence<Others..., T...>;

    private:
        template<typename, typename>
        struct seq_traits;

        template<auto... I, auto... J>
        struct seq_traits<std::index_sequence<I...>, std::index_sequence<J...>>
        {
            struct type
            {
                template<typename... Others>
                using apply = regular_type_sequence<
                    type_sequence::type<I>...,
                    Others...,
                    type_sequence::type<J>...>;
            };
        };

    public:
        template<std::size_t Index, typename... Other>
        using insert_t = details::seq_insert_trait<size(), seq_traits>:: //
            template type<Index>::template apply<Other...>;

        template<std::size_t Index, typename... Other>
        using replace_t = details::seq_rmv_trait<size(), seq_traits>:: //
            template type<Index>::template apply<Other...>;

        template<std::size_t Index>
        using remove_at_t = replace_t<Index>;
    };
}

namespace stdsharp
{
    template<typename T>
    using to_type_sequence = decltype( //
        []<template<typename...> typename Inner, typename... U>(const Inner<U...>&)
        {
            return type_sequence<U...>{}; //
        }(std::declval<T>())
    );
}

namespace stdsharp::details
{
    template<typename>
    struct reverse_type_sequence;

    template<typename... T>
    struct reverse_type_sequence<type_sequence<T...>>
    {
        using seq = type_sequence<T...>;

        template<std::size_t... I>
        static consteval regular_type_sequence<typename seq::template type<seq::size() - I - 1>...>
            impl(std::index_sequence<I...>);

        using type = decltype(impl(std::make_index_sequence<seq::size()>{}));
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

namespace stdsharp::type_sequence_algo
{
    template<typename Seq>
    using reverse_t = details::reverse_type_sequence<Seq>::type;

    template<typename Seq, typename Comp = std::ranges::equal_to>
    using unique_t = details::unique_type_sequence<Seq, Comp>::type;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::type_sequence<T...>> :
        stdsharp::index_constant<::stdsharp::type_sequence<T...>::size()>
    {
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::type_sequence<T...>>
    {
        using type = typename ::stdsharp::type_sequence<T...>::template type<I>;
    };
}

#include "../compilation_config_out.h"