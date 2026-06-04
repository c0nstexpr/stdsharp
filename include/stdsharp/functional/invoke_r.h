#pragma once
#include "../macros.h"

#include <functional>
#include <type_traits>

namespace stdsharp
{
    template<typename Func, typename ReturnT, typename... Args>
    concept invocable_r = std::is_invocable_r_v<ReturnT, Func, Args...>;

    template<typename Func, typename ReturnT, typename... Args>
    concept nothrow_invocable_r = std::is_nothrow_invocable_r_v<ReturnT, Func, Args...>;

    template<typename ReturnT>
    struct invoke_r_fn
    {
        template<typename... Args, typename Func>
            requires invocable_r<ReturnT, Func, Args...>
        [[nodiscard]] constexpr ReturnT operator()(Func&& func, Args&&... args) const //
            noexcept(nothrow_invocable_r<ReturnT, Func, Args...>)
        {
            return std::invoke_r<ReturnT>(cpp_forward(func), cpp_forward(args)...);
        };
    };

    template<typename ReturnT>
    inline constexpr invoke_r_fn<ReturnT> invoke_r{};
}
