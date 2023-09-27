#pragma once

#include <range/v3/functional/invoke.hpp>

#include "invocables.h"

namespace stdsharp
{
    using ranges::invoke;

    inline constexpr struct empty_invoke_fn
    {
        constexpr empty_t operator()(const auto&...) const noexcept { return {}; }
    } empty_invoke{};

    inline constexpr auto optional_invoke = make_sequenced_invocables(invoke, empty_invoke);

    template<typename... Args>
    concept nothrow_optional_invocable = noexcept(optional_invoke(std::declval<Args>()...));

    template<bool Condition>
    struct conditional_invoke_fn
    {
        template<std::invocable Func>
            requires(Condition)
        constexpr decltype(auto) operator()(Func&& func, const auto& = empty_invoke) const
            noexcept(nothrow_invocable<Func>)
        {
            return func();
        }

        template<std::invocable Func = empty_invoke_fn>
        constexpr decltype(auto) operator()(const auto&, Func&& func = empty_invoke) const
            noexcept(nothrow_invocable<Func>)
        {
            return func();
        }
    };

    template<bool Condition>
    inline constexpr conditional_invoke_fn<Condition> conditional_invoke{};

    template<bool Condition, typename T, typename U>
    concept conditional_invocable = std::invocable<conditional_invoke_fn<Condition>, T, U>;

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
            return
#if __cpp_lib_invoke_r >= 202106L
                std::invoke_r<ReturnT>(cpp_forward(func), cpp_forward(args)...)
#else
                static_cast<ReturnT>(std::invoke(cpp_forward(func), cpp_forward(args)...))
#endif
                    ;
        };
    };

    template<typename ReturnT>
    inline constexpr invoke_r_fn<ReturnT> invoke_r{};

    inline constexpr struct projected_invoke_fn
    {
        template<typename Fn, typename Projector, typename... Args>
            requires requires //
        {
            requires(std::invocable<Projector, Args> && ...);
            requires std::invocable<Fn, std::invoke_result_t<Projector, Args>...>;
        }
        constexpr decltype(auto) operator()(Fn&& fn, Projector projector, Args&&... args) const
            noexcept(
                (nothrow_invocable<Projector, Args> && ...) && //
                nothrow_invocable<Fn, std::invoke_result_t<Projector, Args>...> //
            )
        {
            return std::invoke(cpp_forward(fn), std::invoke(projector, cpp_forward(args))...);
        }
    } projected_invoke{};

    template<typename... Args>
    concept projected_invocable = std::invocable<projected_invoke_fn, Args...>;

    template<typename... Args>
    concept projected_nothrow_invocable = nothrow_invocable<projected_invoke_fn, Args...>;
}