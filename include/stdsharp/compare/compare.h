#pragma once

#include <compare>

#include "../concepts/concepts.h"

namespace stdsharp
{
    constexpr bool is_ud(const std::partial_ordering c) noexcept
    {
        return c == std::partial_ordering::unordered;
    }

    template<typename Fn, typename T, typename U, typename Cat = std::partial_ordering>
    concept ordering_predicate =
        std::convertible_to<Cat, std::partial_ordering> && invocable_r<Fn, Cat, T, U>;

    template<typename Fn, typename T, typename U, typename Cat = std::partial_ordering>
    concept nothrow_ordering_predicate =
        ordering_predicate<Fn, T, U, Cat> && nothrow_invocable_r<Fn, Cat, T, U>;
}