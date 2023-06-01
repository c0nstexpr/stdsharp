#pragma once

#include <cmath>

#include "../concepts/concepts.h"

namespace stdsharp
{
    constexpr auto ceil_quotient(const std::integral auto x, decltype(x) y) noexcept
    {
        return (x + y - 1) / y;
    }

    constexpr auto ceil_quotient(const floating_point auto x, decltype(x) y) noexcept
    {
        return ceil(x / y);
    }
}