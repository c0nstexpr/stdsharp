#pragma once

#include "concepts/concepts.h"

namespace stdsharp
{
    template<typename T>
    struct value_wrapper
    {
        using value_type = T;

        T value;

        template<typename... U>
            requires ::std::
                constructible_from<T, U...> // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr value_wrapper(U&&... u) noexcept(concepts::nothrow_constructible_from<T, U...>):
            value(::std::forward<U>(u)...)
        {
        }

        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr operator T&() noexcept { return value; }

        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr operator const T&() const noexcept { return value; }
    };

    template<typename T>
    value_wrapper(T&&) -> value_wrapper<T&&>;
}