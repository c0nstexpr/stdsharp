
#pragma once

#include "value_sequence.h"

namespace stdsharp
{
    namespace details
    {
        template<typename... Types>
        struct type_sequence;
    }

    template<typename... Types>
    using type_sequence = adl_proof_t<details::type_sequence, Types...>;

    namespace details
    {
        struct type_seq_conversion
        {
            template<auto... Values>
            static stdsharp::regular_type_sequence<::meta::_t<decltype(Values)>...>
                from_value_seq(regular_value_sequence<Values...>);

            template<typename... T>
            static regular_value_sequence<(stdsharp::type_constant<T>{})...>
                to_value_seq(regular_type_sequence<T...>);

            template<typename Seq>
            using from_value_seq_t = decltype(from_value_seq(::std::declval<Seq>()));

            template<typename Seq>
            using to_value_seq_t = decltype(to_value_seq(::std::declval<Seq>()));
        };

        struct as_type_sequence
        {
            template<typename... T>
            using invoke = stdsharp::type_sequence<T...>;
        };

        template<typename... Types>
        struct type_sequence :
            private stdsharp::value_sequence<stdsharp::type_constant<Types>{}...>,
            stdsharp::regular_type_sequence<Types...>,
            private type_seq_conversion
        {
        private:
            using base = stdsharp::value_sequence<stdsharp::type_constant<Types>{}...>;

            using type_seq_conversion::from_value_seq_t;
            using type_seq_conversion::to_value_seq_t;

            template<typename... T>
            using regular_type_sequence = stdsharp::regular_type_sequence<T...>;

            template<typename T>
            using type_constant = stdsharp::type_constant<T>;

            template<::std::size_t I>
                requires(I < base::size())
            [[nodiscard]] friend constexpr decltype(auto) get(const type_sequence) noexcept
            {
                return get<I>(base{});
            }

        public:
            using base::adjacent_find;
            using base::all_of;
            using base::any_of;
            using base::contains;
            using base::count;
            using base::count_if;
            using base::count_if_not;
            using base::find;
            using base::find_if;
            using base::find_if_not;
            using base::for_each;
            using base::for_each_n;
            using base::none_of;
            using base::size;

            template<template<typename...> typename T>
            using apply_t = T<Types...>;

            template<::std::size_t I>
            using type = typename base::template value_type<I>::type;

            template<::std::size_t... I>
            using at_t = stdsharp::regular_type_sequence<type<I>...>;

            template<::std::size_t Size>
            using back_t = from_value_seq_t<typename base::template back_t<Size>>;

            template<::std::size_t Size>
            using front_t = from_value_seq_t<typename base::template front_t<Size>>;

            template<typename... Others>
            using append_t = regular_type_sequence<Types..., Others...>;

            template<typename T = void>
            using invoke_fn = typename base::template invoke_fn<T>;

            template<typename T = empty_t>
            static constexpr invoke_fn<T> invoke{};

            template<typename... Others>
            using append_front_t = regular_type_sequence<Others..., Types...>;

            template<::std::size_t Index, typename... Other>
            using insert_t = from_value_seq_t< //
                typename base::template insert_t<
                    Index,
                    static_const_v<type_constant<Other>>... // clang-format off
                >
            >; // clang-format on

            template<::std::size_t... Index>
            using remove_at_t = from_value_seq_t<typename base::template remove_at_t<Index...>>;

            template<::std::size_t Index, typename Other>
            using replace_t =
                from_value_seq_t<typename base::template replace_t<Index, type_constant<Other>{}>>;

            template<::std::size_t From, ::std::size_t Size>
            using select_range_t =
                from_value_seq_t<typename base::template select_range_t<From, Size>>;
        };
    }

    template<typename T>
    using as_type_sequence_t = ::meta::apply<details::as_type_sequence, T>;

    template<typename... T>
    using reverse_type_sequence_t = details::type_seq_conversion::from_value_seq_t<
        reverse_value_sequence_t<type_constant<T>{}...>>;

    template<typename... T>
    using unique_type_sequence_t = details::type_seq_conversion::from_value_seq_t<
        unique_value_sequence_t<type_constant<T>{}...>>;

    template<template<typename...> typename T, typename... U>
        requires ::std::same_as<T<>, type_sequence<>>
    inline constexpr auto enable_tuple_element_by_get<T<U...>> = true;
}

namespace std
{
    template<typename Seq>
        requires same_as<::stdsharp::template_rebind<Seq>, ::stdsharp::type_sequence<>>
    struct tuple_size<Seq> :// NOLINT(cert-dcl58-cpp)
        ::stdsharp::index_constant<Seq::size()>
    {
    };

}