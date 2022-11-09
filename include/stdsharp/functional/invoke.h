#pragma once

#include <range/v3/functional/invoke.hpp>

#include "invocables.h"

namespace stdsharp
{
    inline constexpr struct empty_invoke_fn
    {
        constexpr empty_t operator()(const auto&...) const noexcept { return {}; }
    } empty_invoke{};

    inline constexpr sequenced_invocables optional_invoke{::ranges::invoke, empty_invoke};

    template<typename... Args>
    concept nothrow_optional_invocable = noexcept(optional_invoke(::std::declval<Args>()...));

    template<bool Condition>
    struct conditional_invoke_fn
    {
        template<::std::invocable Func>
            requires(Condition)
        constexpr decltype(auto) operator()(Func&& func, const auto& = empty_invoke) const
            noexcept(nothrow_invocable<Func>)
        {
            return func();
        }

        template<::std::invocable Func = empty_invoke_fn>
        constexpr decltype(auto) operator()(const auto&, Func&& func = empty_invoke) const
            noexcept(nothrow_invocable<Func>)
        {
            return func();
        }
    };

    template<bool Condition>
    inline constexpr conditional_invoke_fn<Condition> conditional_invoke{};

    template<bool Condition, typename T, typename U>
    concept conditional_invocable = ::std::invocable<conditional_invoke_fn<Condition>, T, U>;

    template<bool Condition, typename T, typename U>
    concept nothrow_conditional_invocable =
        nothrow_invocable<conditional_invoke_fn<Condition>, T, U>;

    template<typename ReturnT>
    struct invoke_r_fn
    {
        template<typename... Args, invocable_r<ReturnT, Args...> Func>
        [[nodiscard]] constexpr ReturnT operator()(Func&& func, Args&&... args) const
            noexcept(nothrow_invocable_r<Func, ReturnT, Args...>)
        {
            return ::std::invoke(::std::forward<Func>(func), ::std::forward<Args>(args)...);
        };
    };

    template<decay_same_as<void> ReturnT>
    struct invoke_r_fn<ReturnT>
    {
        template<typename... Args, ::std::invocable<Args...> Func>
        constexpr void operator()(Func&& func, Args&&... args) const
            noexcept(nothrow_invocable<Func, Args...>)
        {
            ::std::invoke(::std::forward<Func>(func), ::std::forward<Args>(args)...);
        };
    };

    template<typename ReturnT>
    inline constexpr invoke_r_fn<ReturnT> invoke_r{};
}