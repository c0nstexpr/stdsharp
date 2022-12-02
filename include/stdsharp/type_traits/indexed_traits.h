#pragma once

#include "core_traits.h"

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
                [[nodiscard]] friend constexpr stdsharp::type_constant<
                    T> get_type(const t&) noexcept
                {
                    return {};
                }
            };
        };
    }

    template<typename T, ::std::size_t I>
    using indexed_type = typename details::indexed_type<T, I>::t;

    template<typename T, ::std::size_t I>
    struct indexed_value : value_wrapper<T>, indexed_type<T, I>
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
        return static_cast<const_ T ref_>(this_.value);                                         \
    }

        STDSHARP_GET(, &)
        STDSHARP_GET(const, &)
        STDSHARP_GET(, &&)
        STDSHARP_GET(const, &&)

#undef STDSHARP_GET
    };

    template<::std::size_t I, typename T>
    constexpr indexed_value<::std::decay_t<T>, I> make_indexed_value(T&& t)
    {
        return ::std::forward<T>(t);
    }

    namespace details
    {
        template<template<typename, auto> typename Indexed, typename... T>
        struct indexed_types
        {
            template<::std::size_t... Index>
            struct inherited : Indexed<T, Index>...
            {
                inherited() = default;

                template<typename... U>
                    requires(::std::constructible_from<Indexed<T, Index>, U> && ...)
                constexpr inherited(U&&... u
                ) noexcept((nothrow_constructible_from<Indexed<T, Index>, U> && ...)):
                    Indexed<T, Index>(::std::forward<U>(u))...
                {
                }
            };

            template<typename = ::std::index_sequence_for<T...>>
            struct t;

            template<::std::size_t... Index>
            struct t<::std::index_sequence<Index...>> : inherited<Index...>
            {
                template<::std::size_t I>
                using type = typename decltype(get_type<I>(inherited<Index...>{}))::type;
            };
        };
    }

    template<typename... T>
    using indexed_types = typename details::indexed_types<indexed_type, T...>::template t<>;

    template<typename... T>
    struct indexed_values : details::indexed_types<indexed_value, T...>::template t<>
    {
    };

    template<typename... T>
    indexed_values(T&&...) -> indexed_values<::std::decay_t<T>...>;
}