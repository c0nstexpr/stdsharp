#pragma once

#include "../concepts/concepts.h"

namespace stdsharp
{
    template<typename T>
    struct value_wrapper
    {
        using value_type = T;

        T value;

        value_wrapper() = default;

        template<typename U>
            requires ::std::constructible_from<T, U>
        constexpr value_wrapper(U&& u) noexcept(nothrow_constructible_from<T, U>):
            value(::std::forward<U>(u))
        {
        }

#define STDSHARP_OPERATOR(const_, ref)                    \
    constexpr operator const_ T ref() const_ ref noexcept \
    {                                                     \
        return static_cast<const_ T ref>(value);          \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &) STDSHARP_OPERATOR(, &&) STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename T>
    value_wrapper(T&&) -> value_wrapper<T&&>;
}