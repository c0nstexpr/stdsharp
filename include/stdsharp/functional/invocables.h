//
// Created by BlurringShadow on 2021-10-15.
//
#pragma once

#include "../type_traits/core_traits.h"

namespace stdsharp::functional
{
    template<typename Func>
    class invocable : value_wrapper<Func>
    {
        using base = value_wrapper<Func>;

    public:
        using base::base;
#define STDSHARP_OPERATOR(const_, ref)                                             \
    template<typename... Args>                                                     \
        requires ::std::invocable<const_ Func ref, Args...>                        \
    constexpr decltype(auto) operator()(Args&&... args)                            \
        const_ ref noexcept(concepts::nothrow_invocable<const_ Func ref, Args...>) \
    {                                                                              \
        return ::std::invoke(                                                      \
            static_cast<const_ Func ref>(base::value),                             \
            ::std::forward<Args>(args)...                                          \
        );                                                                         \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    namespace details
    {
        template<typename... Func>
        struct trivial
        {
            template<typename... Args>
            static constexpr auto value = []
            {
                constexpr auto size = sizeof...(Func);
                ::std::size_t i = 0;
                ::std::size_t target = size;

                for(const auto v : {::std::invocable<Func, Args...>...})
                {
                    if(v)
                    {
                        if(target == size) target = i;
                        else return size;
                    }

                    ++i;
                }
                return target;
            }();
        };

        template<typename... Func>
        struct sequenced
        {
            template<typename... Args>
            static constexpr auto value = []
            {
                ::std::size_t i = 0;
                for(const auto v : {::std::invocable<Func, Args...>...})
                {
                    if(v) break;
                    ++i;
                }
                return i;
            }();
        };

        template<template<typename...> typename, typename, typename...>
        struct invocables;

        template<template<typename...> typename Selector, ::std::size_t... I, typename... Func>
        struct invocables<Selector, ::std::index_sequence<I...>, Func...> :
            type_traits::indexed_type<invocable<Func>, I>...
        {
#define STDSHARP_OPERATOR(const_, ref)                                              \
    template<                                                                       \
        typename... Args,                                                           \
        typename This = const_ invocables ref,                                      \
        auto Index = Selector<const_ Func ref...>::template value<Args...>,         \
        typename Invocable = decltype(get<Index>(::std::declval<This>()))>          \
        requires ::std::invocable<Invocable, Args...>                               \
    constexpr decltype(auto) operator()(Args&&... args)                             \
        const_ ref noexcept(concepts::nothrow_invocable<Invocable, Args...>)        \
    {                                                                               \
        return get<Index>(static_cast<This>(*this))(::std::forward<Args>(args)...); \
    }

            STDSHARP_OPERATOR(, &)
            STDSHARP_OPERATOR(const, &)
            STDSHARP_OPERATOR(, &&)
            STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
        };
    }

    template<typename... Func>
    struct trivial_invocables :
        details::invocables<details::trivial, ::std::index_sequence_for<Func...>, Func...>
    {
        using base =
            details::invocables<details::trivial, ::std::index_sequence_for<Func...>, Func...>;

        using base::operator();
    };

    template<typename... Func>
    trivial_invocables(Func&&...) -> trivial_invocables<::std::decay_t<Func>...>;

    template<typename... Func>
    struct sequenced_invocables :
        details::invocables<details::sequenced, ::std::index_sequence_for<Func...>, Func...>
    {
        using details::invocables<details::sequenced, ::std::index_sequence_for<Func...>, Func...>::
            operator();
    };

    template<typename... Func>
    sequenced_invocables(Func&&...) -> sequenced_invocables<::std::decay_t<Func>...>;

    template<typename Func>
    class nodiscard_invocable : value_wrapper<Func>
    {
        using base = value_wrapper<Func>;

    public:
        using base::base;

#define STDSHARP_OPERATOR(const_, ref)                                             \
    template<typename... Args>                                                     \
        requires ::std::invocable<const_ Func ref, Args...>                        \
    [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args)              \
        const_ ref noexcept(concepts::nothrow_invocable<const_ Func ref, Args...>) \
    {                                                                              \
        return ::std::invoke(                                                      \
            static_cast<const_ Func ref>(base::value),                             \
            ::std::forward<Args>(args)...                                          \
        );                                                                         \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename Func>
    nodiscard_invocable(Func&& func) -> nodiscard_invocable<::std::decay_t<Func>>;

    inline constexpr struct make_trivial_invocables_fn
    {
        template<typename... Invocable>
            requires requires { trivial_invocables{::std::declval<Invocable>()...}; }
        [[nodiscard]] constexpr auto operator()(Invocable&&... invocable) const
            noexcept(noexcept(trivial_invocables{::std::declval<Invocable>()...}))
        {
            return trivial_invocables{::std::forward<Invocable>(invocable)...};
        }
    } make_trivial_invocables{};

    inline constexpr struct make_sequenced_invocables_fn
    {
        template<typename... Invocable>
            requires requires { sequenced_invocables{::std::declval<Invocable>()...}; }
        [[nodiscard]] constexpr auto operator()(Invocable&&... invocable) const
            noexcept(noexcept(sequenced_invocables{::std::declval<Invocable>()...}))
        {
            return sequenced_invocables{::std::forward<Invocable>(invocable)...};
        }
    } make_sequenced_invocables{};
}
