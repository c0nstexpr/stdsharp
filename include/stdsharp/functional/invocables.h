//

#pragma once

#include <algorithm>
#include <functional>

#include "../type_traits/indexed_traits.h"

namespace stdsharp
{
    namespace details
    {
        template<typename... Func>
        struct invocables_traits
        {
            using indexed_values = stdsharp::indexed_values<Func...>;

            template<typename T, std::size_t I>
            struct indexed_operator
            {
            private:
                using func = std::tuple_element_t<I, indexed_values>;

            public:
#define STDSHARP_OPERATOR(const_, ref)                                                      \
    template<typename... Args, typename Fn = const_ func ref>                               \
        requires ::std::invocable<Fn, Args...>                                              \
    constexpr decltype(auto) operator()(Args&&... args)                                     \
        const_ ref noexcept(nothrow_invocable<Fn, Args...>)                                 \
    {                                                                                       \
        return std::invoke(                                                                 \
            cpo::get_element<I>(                                                            \
                static_cast<const_ indexed_values ref>(static_cast<const_ type ref>(*this)) \
            ),                                                                              \
            cpp_forward(args)...                                                            \
        );                                                                                  \
    }

                STDSHARP_OPERATOR(, &)
                STDSHARP_OPERATOR(const, &)
                STDSHARP_OPERATOR(, &&)
                STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
            };

            template<typename = std::index_sequence_for<Func...>>
            struct impl;

            using type = impl<>;

            template<std::size_t... I>
            struct impl<std::index_sequence<I...>> :
                indexed_operator<indexed_values, I>...,
                stdsharp::indexed_values<Func...>
            {
                using stdsharp::indexed_values<Func...>::indexed_values;
                using indexed_operator<indexed_values, I>::operator()...;
            };
        };
    }

    template<typename... Func>
    struct invocables : ::meta::_t<details::invocables_traits<Func...>>
    {
    private:
        using m_base = ::meta::_t<details::invocables_traits<Func...>>;

    public:
        using m_base::m_base;
    };

    template<typename... T>
    invocables(T&&...) -> invocables<std::decay_t<T>...>;

    template<std::size_t Index>
    struct invoke_at_fn
    {
        template<typename T, typename... Args>
            requires std::invocable<get_element_t<Index, T>, Args...>
        constexpr decltype(auto) operator()(T && t, Args&&... args) const
            noexcept(nothrow_invocable<get_element_t<Index, T>, Args...>)
        {
            return cpo::get_element<Index>(t)(cpp_forward(args)...);
        }
    };

    template<auto Index>
    inline constexpr invoke_at_fn<Index> invoke_at{};

    template<template<typename> typename Predicator>
    struct invoke_first_fn
    {
    private:
        template<typename T, typename... Args, std::size_t I = 0>
        static constexpr std::size_t find_first(const index_constant<I> = {}) noexcept
        {
            if constexpr(I < std::tuple_size_v<std::decay_t<T>>)
                if constexpr(Predicator<get_element_t<I, T>>::template value<Args...>) return I;
                else return find_first<T, Args...>(index_constant<I + 1>{});
            else return static_cast<std::size_t>(-1);
        }

        template<typename T, typename... Args>
        static constexpr auto index = find_first<T, Args...>();

        template<typename T, typename... Args>
            requires(index<T, Args...> != -1)
        using invoke_at_t = invoke_at_fn<index<T, Args...>>;

    public:
        template<typename T, typename... Args>
            requires requires { typename invoke_at_t<T, Args...>; }
        constexpr decltype(auto) operator()(T && t, Args&&... args) const
            noexcept(nothrow_invocable<invoke_at_t<T, Args...>, T, Args...>)
        {
            return invoke_at_t<T, Args...>{}(t, cpp_forward(args)...);
        }
    };

    template<template<typename> typename Predicator>
    inline constexpr invoke_first_fn<Predicator> invoke_first{};

    namespace details
    {
        template<typename Func>
        struct sequenced_invocables_predicate
        {
            template<typename... Args>
            static constexpr auto value = std::invocable<Func, Args...>;
        };
    }

    using sequenced_invoke_fn = invoke_first_fn<details::sequenced_invocables_predicate>;

    inline constexpr sequenced_invoke_fn sequenced_invoke{};

    template<typename... Invocable>
    struct sequenced_invocables : invocables<Invocable...>
    {
        using base = invocables<Invocable...>;

        using invocables<Invocable...>::invocables;

#define STDSHARP_OPERATOR(const_, ref)                                          \
    template<                                                                   \
        typename... Args,                                                       \
        typename Base = const_ base ref,                                        \
        std::invocable<Base, Args...> Fn = sequenced_invoke_fn>                 \
    constexpr decltype(auto) operator()(Args&&... args)                         \
        const_ ref noexcept(nothrow_invocable<Fn, Base, Args...>)               \
    {                                                                           \
        return Fn{}(static_cast<const_ base ref>(*this), cpp_forward(args)...); \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename... T>
    sequenced_invocables(T&&...) -> sequenced_invocables<std::decay_t<T>...>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::invocables<T...>> :
        ::std::tuple_size<::stdsharp::indexed_types<T...>>
    {
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::invocables<T...>> :
        ::std::tuple_element<I, ::stdsharp::indexed_types<T...>>
    {
    };

    template<typename... T>
    struct tuple_size<::stdsharp::sequenced_invocables<T...>> :
        ::std::tuple_size<::stdsharp::invocables<T...>>
    {
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::sequenced_invocables<T...>> :
        ::std::tuple_element<I, ::stdsharp::invocables<T...>>
    {
    };
}