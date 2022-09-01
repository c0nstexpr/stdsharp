#pragma once

#include <range/v3/functional/invoke.hpp>

#include "invocables.h"

namespace stdsharp::functional
{
    inline constexpr struct empty_invoke_fn
    {
        constexpr auto operator()(const auto&...) const noexcept
        {
            return type_traits::empty; //
        }
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
            noexcept(concepts::nothrow_invocable<Func>)
        {
            return func();
        }

        template<::std::invocable Func = empty_invoke_fn>
        constexpr decltype(auto) operator()(const auto&, Func&& func = empty_invoke) const
            noexcept(concepts::nothrow_invocable<Func>)
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
        concepts::nothrow_invocable<conditional_invoke_fn<Condition>, T, U>;

    template<concepts::not_same_as<void> ReturnT>
    struct invoke_r_fn
    {
        template<typename... Args, concepts::invocable_r<ReturnT, Args...> Func>
        [[nodiscard]] constexpr ReturnT operator()(Func&& func, Args&&... args) const
            noexcept(concepts::nothrow_invocable_r<Func, ReturnT, Args...>) //
        {
            return static_cast<ReturnT>(
                ::std::invoke(::std::forward<Func>(func), ::std::forward<Args>(args)...) //
            );
        };
    };

    template<typename ReturnT>
    inline constexpr invoke_r_fn<ReturnT> invoke_r{};
}