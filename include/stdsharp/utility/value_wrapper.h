#pragma once

#include "../concepts/concepts.h"

namespace stdsharp
{
    template<typename T>
    struct value_wrapper
    {
        using value_type = T;

        value_wrapper() = default;

        template<typename... U>
            requires ::std::constructible_from<T, U...>
        constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
            v(::std::forward<U>(u)...)
        {
        }

#define STDSHARP_OPERATOR(const_, ref)                             \
    constexpr const_ T ref value() const_ ref noexcept             \
    {                                                              \
        return static_cast<const_ T ref>(v);                       \
    }                                                              \
    constexpr explicit operator const_ T ref() const_ ref noexcept \
    {                                                              \
        return value();                                            \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR

        T v;
    };
}