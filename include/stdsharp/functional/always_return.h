#pragma once

#include "../macros.h"

#include <functional>

namespace stdsharp
{
    inline constexpr struct always_return_fn
    {
        constexpr decltype(auto) operator()(auto&& v, const auto&... /*unused*/) const noexcept
        {
            return cpp_forward(v);
        }
    } always_return{};

    inline constexpr auto always_true = std::bind_front(always_return, true);

    using always_true_fn = decltype(always_true);

    inline constexpr auto always_false = std::bind_front(always_return, false);

    using always_false_fn = decltype(always_false);

    template<typename T>
    inline constexpr auto always_default = std::bind_front(always_return, T{});

    template<typename T>
    using always_default_fn = decltype(always_default<T>);
}