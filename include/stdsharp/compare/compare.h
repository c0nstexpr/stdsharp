#pragma once

#include <compare>

namespace stdsharp
{
    constexpr bool is_ud(const ::std::partial_ordering c) noexcept
    {
        return c == ::std::partial_ordering::unordered;
    }
}