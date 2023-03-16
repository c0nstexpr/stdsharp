
#pragma once

#include "value_sequence.h"

namespace stdsharp
{
    namespace details
    {
        template<typename, typename...>
        struct type_sequence;
    }

    template<typename... Types>
    using type_sequence = adl_proof_t<
        details::type_sequence,
        stdsharp::value_sequence<stdsharp::type_constant<Types>{}...>,
        Types... // clang-format off
    >; // clang-format on

    namespace details
    {
        struct type_seq_conversion
        {
            template<typename Seq>
            using from_value_seq = decltype( //
                []<auto... Values>(const regular_value_sequence<Values...>) //
                {
                    return stdsharp::regular_type_sequence<::meta::_t<decltype(Values)>...>{};
                }(Seq{})
            );
        };

        struct as_type_sequence
        {
            template<typename... T>
            using invoke = stdsharp::type_sequence<T...>;
        };

        template<typename Base, typename... Types>
        struct type_sequence :
            private Base,
            stdsharp::regular_type_sequence<Types...>,
            private type_seq_conversion
        {
        private:
            using type_seq_conversion::from_value_seq;

            template<typename... T>
            using regular_type_sequence = stdsharp::regular_type_sequence<T...>;

            template<typename T>
            using type_constant = stdsharp::type_constant<T>;

            template<::std::size_t I>
                requires requires { requires I < Base::size(); }
            [[nodiscard]] friend constexpr decltype(auto) get(const type_sequence) noexcept
            {
                return get<I>(Base{});
            }

        public:
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

            template<::std::size_t I>
            using type = typename Base::template value_type<I>::type;

            template<::std::size_t... I>
            using at_t = stdsharp::regular_type_sequence<type<I>...>;

            template<::std::size_t Size>
            using back_t = from_value_seq<typename Base::template back_t<Size>>;

            template<::std::size_t Size>
            using front_t = from_value_seq<typename Base::template front_t<Size>>;

            template<typename... Others>
            using append_t = regular_type_sequence<Types..., Others...>;

            template<typename T = void>
            using invoke_fn = typename Base::template invoke_fn<T>;

            template<typename T = empty_t>
            static constexpr invoke_fn<T> invoke{};

            template<typename... Others>
            using append_front_t = regular_type_sequence<Others..., Types...>;

            template<::std::size_t Index, typename... Other>
            using insert_t = from_value_seq< //
                typename Base::template insert_t<
                    Index,
                    static_const_v<type_constant<Other>>... // clang-format off
                >
            >; // clang-format on

            template<::std::size_t... Index>
            using remove_at_t = from_value_seq<typename Base::template remove_at_t<Index...>>;

            template<::std::size_t Index, typename Other>
            using replace_t =
                from_value_seq<typename Base::template replace_t<Index, type_constant<Other>{}>>;

            template<::std::size_t From, ::std::size_t Size>
            using select_range_t =
                from_value_seq<typename Base::template select_range_t<From, Size>>;
        };
    }

    template<typename T>
    using to_type_sequence = ::meta::apply<details::as_type_sequence, T>;

    template<typename... T>
    using reverse_type_sequence = details::type_seq_conversion:: //
        from_value_seq<reverse_value_sequence<::std::type_identity<T>{}...>>;

    template<typename... T>
    using unique_type_sequence = details::type_seq_conversion:: //
        from_value_seq<unique_value_sequence<basic_type_constant<T>{}...>>;
}

namespace std
{
    template<typename Seq>
        requires same_as<::stdsharp::template_rebind<Seq>, ::stdsharp::type_sequence<>>
    struct tuple_size<Seq> : ::stdsharp::index_constant<Seq::size()>
    {
    };

    template<::std::size_t I, typename Seq>
        requires same_as<::stdsharp::template_rebind<Seq>, ::stdsharp::type_sequence<>>
    struct tuple_element<I, Seq>
    {
        using type = typename Seq::template type<I>;
    };
}