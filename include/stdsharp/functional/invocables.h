#pragma once

#include <algorithm>
#include <functional>

#include "../type_traits/indexed_traits.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename... Func>
    struct invocables_traits
    {
        using indexed_values = stdsharp::indexed_values<Func...>;

        template<typename = std::index_sequence_for<Func...>>
        struct impl;

        using type = impl<>;

        template<typename T, std::size_t I>
        struct indexed_operator
        {
        private:
            using func = std::tuple_element_t<I, indexed_values>;

        public:
            template<typename Self, typename... Args, typename Fn = forward_cast_t<Self, func>>
                requires ::std::invocable<Fn, Args...> &&
                std::invocable<forward_cast_fn<Self, type>, Self>
            constexpr decltype(auto) operator()(this Self&& self, Args&&... args)
                noexcept(nothrow_invocable<Fn, Args...>)
            {
                return invoke(
                    cpo::get_element<I>(forward_cast<Self, type>(self)),
                    cpp_forward(args)...
                );
            }
        };

        template<std::size_t... I>
        struct STDSHARP_EBO impl<std::index_sequence<I...>> :
            indexed_values,
            indexed_operator<indexed_values, I>...
        {
            using indexed_values::indexed_values;
            using indexed_operator<indexed_values, I>::operator()...;
        };
    };
}

namespace stdsharp
{
    template<typename... Func>
    struct invocables : details::invocables_traits<Func...>::type
    {
    private:
        using m_invocables = details::invocables_traits<Func...>::type;

    public:
        template<typename... Args>
            requires std::constructible_from<m_invocables, Args...>
        constexpr invocables(Args&&... args)
            noexcept(nothrow_constructible_from<m_invocables, Args...>):
            m_invocables(cpp_forward(args)...)
        {
        }

        invocables() = default;
    };

    template<typename... T>
    invocables(T&&...) -> invocables<std::decay_t<T>...>;

    template<std::size_t Index>
    struct invoke_at_fn
    {
        template<typename T, typename... Args>
            requires std::invocable<get_element_t<Index, T>, Args...>
        constexpr decltype(auto) operator()(T&& t, Args&&... args) const
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
        static constexpr std::size_t find_first(const index_constant<I> /*unused*/ = {}) noexcept
        {
            if constexpr( //
                requires {
                    requires I < std::tuple_size_v<std::decay_t<T>>;
                    typename get_element_t<I, T>;
                } //
            )
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
        constexpr decltype(auto) operator()(T&& t, Args&&... args) const
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

        using base::base;

        sequenced_invocables() = default;

        template<typename Self, typename... Args, typename Base = forward_cast_t<Self, base>>
            requires std::invocable<sequenced_invoke_fn, Base, Args...>
        constexpr decltype(auto) operator()(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<sequenced_invoke_fn, Base, Args...>)
        {
            return sequenced_invoke(
                forward_cast<Self, sequenced_invocables, base>(self),
                cpp_forward(args)...
            );
        }
    };

    template<typename... T>
    sequenced_invocables(T&&...) -> sequenced_invocables<std::decay_t<T>...>;

    template<typename Func>
    struct nodiscard_invocable : invocables<Func>
    {
        using base = invocables<Func>;

        using base::base;

        template<typename Self, typename... Args, typename Base = forward_cast_t<Self, base>>
            requires std ::invocable<Base, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<Base, Args...>)
        {
            return forward_cast<Self, nodiscard_invocable, base>(self)(cpp_forward(args)...);
        }
    };

    template<typename Func>
    nodiscard_invocable(Func&&) -> nodiscard_invocable<std::decay_t<Func>>;
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

#include "../compilation_config_out.h"