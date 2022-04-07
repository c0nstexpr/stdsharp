//
// Created by BlurringShadow on 2021-10-15.
//
#pragma once
#include "type_traits/core_traits.h"
#include "utility/value_wrapper.h"
#include "utility/pack_get.h"

namespace stdsharp::functional
{
    inline constexpr struct nodiscard_tag_t
    {
    } nodiscard_tag;

    inline namespace invoke_selector
    {
        template<typename... Func>
        struct trivial
        {
            template<typename... Args>
            static constexpr auto value = []
            {
                constexpr auto size = sizeof...(Func);
                ::std::size_t i = 0;
                ::std::size_t target = sizeof...(Func);
                for(const auto v : {::std::invocable<Func, Args...>...})
                {
                    if(v)
                        if(target == size) target = i;
                        else
                        {
                            target = size;
                            break;
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
    }

    template<template<typename...> typename Selector, typename... Func>
    struct invocables : private value_wrapper<Func>...
    {
        invocables() = default;

        template<typename... Args>
            requires(::std::constructible_from<value_wrapper<Func>, Args>&&...)
        constexpr invocables(Args&&... args) //
            noexcept((concepts::nothrow_constructible_from<value_wrapper<Func>, Args> && ...)):
            value_wrapper<Func>(::std::forward<Args>(args))...
        {
        }

#define BS_OPERATOR(const_, ref)                                                            \
    template<typename... Args>                                                              \
    requires ::std::invocable<                                                              \
        pack_get_t<Selector<Func...>::template value<Args...>, const_ Func ref...>,         \
        Args...> constexpr decltype(auto)                                                   \
        operator()(Args&&... args) const_ ref noexcept(                                     \
            concepts::nothrow_invocable<                                                    \
                pack_get_t<Selector<Func...>::template value<Args...>, const_ Func ref...>, \
                Args...>)                                                                   \
    {                                                                                       \
        return ::std::invoke(                                                               \
            pack_get<Selector<Func...>::template value<Args...>>(                           \
                static_cast<const_ Func ref>(value_wrapper<Func>::value)...),               \
            ::std::forward<Args>(args)...);                                                 \
    }

        BS_OPERATOR(, &)
        BS_OPERATOR(const, &)
        BS_OPERATOR(, &&)
        BS_OPERATOR(const, &&)
#undef BS_OPERATOR
    };

    template<typename... Func>
    struct trivial_invocables : invocables<invoke_selector::trivial, Func...>
    {
        using base = invocables<invoke_selector::trivial, Func...>;
        using base::base;
    };

    template<typename... Func>
    trivial_invocables(Func&&...) -> trivial_invocables<::std::decay_t<Func>...>;

    template<typename... Func>
    struct sequenced_invocables : invocables<invoke_selector::sequenced, Func...>
    {
        using base = invocables<invoke_selector::sequenced, Func...>;
        using base::base;
    };

    template<typename... Func>
    sequenced_invocables(Func&&...) -> sequenced_invocables<::std::decay_t<Func>...>;

    template<typename Func>
    class nodiscard_invocable : value_wrapper<Func>
    {
        using base = value_wrapper<Func>;

    public:
        using base::base;

#define BS_OPERATOR(const_, ref)                                                       \
    template<typename... Args>                                                         \
        requires ::std::invocable<const_ Func ref, Args...>                            \
    [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args)                  \
        const_ ref noexcept(concepts::nothrow_invocable<const_ Func ref, Args...>)     \
    {                                                                                  \
        return ::std::invoke(                                                          \
            static_cast<const_ Func ref>(base::value), ::std::forward<Args>(args)...); \
    }

        BS_OPERATOR(, &)
        BS_OPERATOR(const, &)
        BS_OPERATOR(, &&)
        BS_OPERATOR(const, &&)
#undef BS_OPERATOR
    };

    template<typename Func>
    nodiscard_invocable(Func&& func) -> nodiscard_invocable<::std::decay_t<Func>>;

    inline constexpr struct make_trivial_invocables_fn
    {
        template<typename... Invocable>
            requires requires { trivial_invocables{::std::declval<Invocable>()...}; }
        [[nodiscard]] constexpr auto operator()(Invocable&&... invocable) const //
            noexcept(noexcept(trivial_invocables{::std::declval<Invocable>()...}))
        {
            return trivial_invocables{::std::forward<Invocable>(invocable)...};
        }

        template<typename... Invocable>
            requires requires
            {
                nodiscard_invocable{trivial_invocables{::std::declval<Invocable>()...}};
            }
        [[nodiscard]] constexpr auto operator()(nodiscard_tag_t, Invocable&&... invocable) const //
            noexcept(noexcept(nodiscard_invocable{
                trivial_invocables{::std::declval<Invocable>()...}}))
        {
            return nodiscard_invocable{trivial_invocables{::std::forward<Invocable>(invocable)...}};
        }
    } make_trivial_invocables{};

    inline constexpr struct make_sequenced_invocables_fn
    {
        template<typename... Invocable>
            requires requires { sequenced_invocables{::std::declval<Invocable>()...}; }
        [[nodiscard]] constexpr auto operator()(Invocable&&... invocable) const //
            noexcept(noexcept(sequenced_invocables{::std::declval<Invocable>()...}))
        {
            return sequenced_invocables{::std::forward<Invocable>(invocable)...};
        }

        template<typename... Invocable>
            requires requires
            {
                nodiscard_invocable{sequenced_invocables{::std::declval<Invocable>()...}};
            }
        [[nodiscard]] constexpr auto operator()(nodiscard_tag_t, Invocable&&... invocable) const //
            noexcept( //
                noexcept( //
                    nodiscard_invocable{sequenced_invocables{::std::declval<Invocable>()...}} //
                    // clang-format off
                )
            ) // clang-format on
        {
            return nodiscard_invocable{
                sequenced_invocables{::std::forward<Invocable>(invocable)...} //
            };
        }
    } make_sequenced_invocables{};
}