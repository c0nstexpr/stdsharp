#pragma once

#include "core_traits.h"
#include "../utility/invocable.h"
#include "../compilation_config_in.h"

namespace stdsharp
{
    template<template<auto> typename T, auto... V>
    struct indexed_trait : T<V>...
    {
    };

    namespace details
    {
        template<template<auto> typename, typename>
        struct to_indexed_trait;

        template<template<auto> typename T, auto... V>
        struct to_indexed_trait<T, ::std::index_sequence<V...>>
        {
            using type = indexed_trait<T, V...>;
        };
    }

    template<template<auto> typename T, typename Seq>
    using to_indexed_trait = details::to_indexed_trait<T, Seq>;

    template<template<auto> typename T, ::std::size_t Size>
    using make_indexed_trait = to_indexed_trait<T, ::std::make_index_sequence<Size>>;

    template<template<auto> typename T, typename... Ts>
    using indexed_trait_for = make_indexed_trait<T, sizeof...(Ts)>;

    namespace details
    {
        template<typename T, ::std::size_t I>
        struct indexed_type
        {
            struct t
            {
                using type = T;

            private:
                template<::std::size_t Index = I>
                    requires(I == Index)
                [[nodiscard]] friend constexpr stdsharp::type_constant<T> get_type(const t) noexcept
                {
                    return {};
                }
            };
        };
    }

    template<typename T, ::std::size_t I>
    using indexed_type = typename details::indexed_type<T, I>::t;

    template<typename T, ::std::size_t I>
    struct STDSHARP_EBO indexed_value : value_wrapper<T>, indexed_type<T, I>
    {
        using base = value_wrapper<T>;

        using base::value;
        using base::base;

    private:
#define STDSHARP_GET(const_, ref_)                                                              \
    template<::std::size_t Index = I>                                                           \
        requires(I == Index)                                                                    \
    [[nodiscard]] friend constexpr decltype(auto) get(const_ indexed_value ref_ this_) noexcept \
    {                                                                                           \
        return this_.value();                                                                   \
    }

        STDSHARP_GET(, &)
        STDSHARP_GET(const, &)
        STDSHARP_GET(, &&)
        STDSHARP_GET(const, &&)

#undef STDSHARP_GET
    };

    template<::std::size_t I>
    inline constexpr nodiscard_invocable make_indexed_value( //
        []<typename T,
           ::std::constructible_from<T> IndexedValue = indexed_value<::std::decay_t<T>, I>> //
        (T&& t) noexcept(nothrow_constructible_from<IndexedValue, T>) //
        {
            return ::std::forward<T>(t); //
        }
    );

    namespace details
    {
        template<template<typename, auto> typename Indexed, typename... T>
        struct indexed_types
        {
            template<::std::size_t... Index>
            struct STDSHARP_EBO inherited : Indexed<T, Index>...
            {
                inherited() = default;

                template<typename... U>
                    requires(::std::constructible_from<Indexed<T, Index>, U> && ...)
                constexpr inherited(U&&... u) //
                    noexcept((nothrow_constructible_from<Indexed<T, Index>, U> && ...)):
                    Indexed<T, Index>(::std::forward<U>(u))...
                {
                }
            };

            template<typename = ::std::index_sequence_for<T...>>
            struct t;

            template<::std::size_t... Index>
            struct STDSHARP_EBO t<::std::index_sequence<Index...>> : inherited<Index...>
            {
            private:
                using m_base = inherited<Index...>;

            public:
                using m_base::m_base;

                template<::std::size_t I>
                using type = typename decltype(get_type<I>(::std::declval<m_base>()))::type;
            };
        };

        template<typename... T>
        using indexed_values = typename details::indexed_types<indexed_value, T...>::template t<>;
    }

    template<typename... T>
    using indexed_types = typename details::indexed_types<indexed_type, T...>::template t<>;

    template<typename... T>
    struct indexed_values : details::indexed_values<T...>
    {
        using details::indexed_values<T...>::indexed_values;
    };

    template<typename... T>
    indexed_values(T&&...) -> indexed_values<::std::decay_t<T>...>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::indexed_values<T...>> : ::stdsharp::index_constant<sizeof...(T)>
    {
    };

    template<::std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::indexed_values<T...>>
    {
        using type = typename ::stdsharp::indexed_values<T...>::template type<I>;
    };

    template<::std::size_t I, typename Seq>
        requires same_as<::stdsharp::template_rebind<Seq>, ::stdsharp::indexed_types<>>
    struct tuple_element<I, Seq>
    {
        using type = typename Seq::template type<I>;
    };
}

#include "../compilation_config_out.h"