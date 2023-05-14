//

#pragma once

#include <algorithm>
#include <functional>

#include "../type_traits/indexed_traits.h"

namespace stdsharp
{
    namespace details
    {
        template<typename T, ::std::size_t I>
        struct indexed_invocable : stdsharp::indexed_value<T, I>
        {
            using base = stdsharp::indexed_value<T, I>;

            using base::base;

#define STDSHARP_OPERATOR(const_, ref)                                      \
    template<typename... Args, ::std::invocable<Args...> Fn = const_ T ref> \
    constexpr decltype(auto) operator()(Args&&... args)                     \
        const_ ref noexcept(nothrow_invocable<Fn, Args...>)                 \
    {                                                                       \
        return base::value()(cpp_forward(args)...);                         \
    }

            STDSHARP_OPERATOR(, &)
            STDSHARP_OPERATOR(const, &)
            STDSHARP_OPERATOR(, &&)
            STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
        };

        template<typename... Func>
        struct invocables
        {
            template<typename = ::std::index_sequence_for<Func...>>
            struct impl;

            using type = impl<>;

            template<::std::size_t... I>
            struct impl<::std::index_sequence<I...>> :
                indexed_invocable<Func, I>...,
                indexed_types<Func...>
            {
                impl() = default;

                template<typename... Args>
                    requires(::std::constructible_from<Func, Args> && ...)
                constexpr impl(Args&&... args) //
                    noexcept((nothrow_constructible_from<Func, Args> && ...)):
                    indexed_invocable<Func, I>(cpp_forward(args))...
                {
                }
            };
        };
    }

    template<typename... Func>
    using invocables = ::meta::_t<details::invocables<Func...>>;

    using make_invocables_fn = make_template_type_fn<invocables>;

    inline constexpr make_invocables_fn make_invocables{};

    template<::std::size_t Index>
    struct invoke_at_fn
    {
        template<typename T, typename... Args>
            requires ::std::invocable<get_element_t<Index, T>, Args...>
        constexpr decltype(auto) operator()(T&& t, Args&&... args) const
            noexcept(nothrow_invocable<get_element_t<Index, T>, Args...>)
        {
            return cpo::get_element<Index>(t)(cpp_forward(args)...);
        }
    };

    template<auto Index>
    inline constexpr invoke_at_fn<Index> invoke_at{};

    template<template<typename...> typename Predicator>
    struct invoke_first_fn
    {
    private:
        template<typename T, typename... Args, ::std::size_t I = 0>
            requires requires //
        {
            requires(Predicator<get_element_t<I, T>, Args...>::value); //
        }
        static constexpr ::std::size_t find_first_impl(const index_constant<I>) noexcept
        {
            return I;
        }

        template<typename T, typename... Args, ::std::size_t I = 0>
        static constexpr ::std::size_t find_first_impl(const index_constant<I>) noexcept
        {
            return find_first<T, Args...>(index_constant<I + 1>{});
        }

        template<typename T, typename... Args, ::std::size_t I = 0>
        static constexpr ::std::size_t find_first(const index_constant<I> = {}) noexcept
        {
            if constexpr(requires { typename get_element_t<I, T>; })
                return find_first_impl<T, Args...>(index_constant<I>{});
            else return static_cast<::std::size_t>(-1);
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

    template<template<typename...> typename Predicator>
    inline constexpr invoke_first_fn<Predicator> invoke_first{};

    namespace details
    {
        template<typename Func, typename... Args>
            requires ::std::invocable<Func, Args...>
        struct sequenced_invocables_predicate
        {
            static constexpr auto value = true;
        };
    }

    using sequenced_invoke_fn = invoke_first_fn<details::sequenced_invocables_predicate>;

    inline constexpr sequenced_invoke_fn sequenced_invoke{};

    template<typename... Invocable>
    struct basic_sequenced_invocables : invocables<Invocable...>
    {
        using base = invocables<Invocable...>;

        using invocables<Invocable...>::invocables;

#define STDSHARP_OPERATOR(const_, ref)                                          \
    template<                                                                   \
        typename... Args,                                                       \
        typename Base = const_ base ref,                                        \
        ::std::invocable<Base, Args...> Fn = sequenced_invoke_fn>               \
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
    basic_sequenced_invocables(T&&...) -> basic_sequenced_invocables<::std::decay_t<T>...>;

    template<typename... Invocable>
    using sequenced_invocables = adl_proof_t<basic_sequenced_invocables, Invocable...>;

    inline constexpr make_template_type_fn<sequenced_invocables> make_sequenced_invocables{};
}