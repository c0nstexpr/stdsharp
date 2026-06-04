#pragma once
#include <cmath> // IWYU pragma: export
#include <concepts>

namespace stdsharp
{
    [[nodiscard]] constexpr auto ceil(const std::integral auto x, decltype(x) y) noexcept
    {
        return (x + y - 1) / y;
    }
}
