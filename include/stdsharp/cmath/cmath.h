#pragma once

#include <cmath>
#include <concepts>

namespace stdsharp
{
    constexpr auto ceil_reminder(const std::integral auto x, decltype(x) y) noexcept
    {
        return (x + y - 1) / y;
    }
}