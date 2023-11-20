#pragma once

#include "utility/cast_to.h"
#include "type_traits/core_traits.h"

namespace stdsharp
{
    template<typename T>
    class default_increase_and_decrease
    {
        [[nodiscard]] friend constexpr auto operator++(T& t, int) //
            noexcept(nothrow_copy_constructible<T>&& noexcept(++t))
            requires std::copy_constructible<T> && requires { ++t; }
        {
            const auto copied = t;
            ++t;
            return copied;
        }

        [[nodiscard]] friend constexpr auto operator--(T& t, int) //
            noexcept(nothrow_copy_constructible<T>&& noexcept(--t))
            requires std::copy_constructible<T> && requires { --t; }
        {
            const auto copied = t;
            --t;
            return copied;
        }
    };

    template<typename T>
    class default_arithmetic_operation : default_increase_and_decrease<T>
    {
        friend constexpr T& operator++(T& t) noexcept(noexcept(t += 1))
            requires requires { t += 1; }
        {
            t += 1;
            return t;
        }

        friend constexpr T& operator--(T& t) noexcept(noexcept(t -= 1))
            requires requires { t -= 1; }
        {
            t -= 1;
            return t;
        }

#define STDSHARP_ARITH_OP(op)                                             \
    [[nodiscard]] friend constexpr T operator op(T t, auto&& u) noexcept( \
        noexcept(t op## = cpp_forward(u))                                 \
    )                                                                     \
        requires requires { t op## = cpp_forward(u); }                    \
    {                                                                     \
        t op## = cpp_forward(u);                                          \
        return t;                                                         \
    }

        STDSHARP_ARITH_OP(+)
        STDSHARP_ARITH_OP(-)
        STDSHARP_ARITH_OP(*)
        STDSHARP_ARITH_OP(/)
        STDSHARP_ARITH_OP(%)
        STDSHARP_ARITH_OP(&)
        STDSHARP_ARITH_OP(|)
        STDSHARP_ARITH_OP(^)
        STDSHARP_ARITH_OP(<<)
        STDSHARP_ARITH_OP(>>)

        [[nodiscard]] friend constexpr decltype(auto) operator-(const T & t) //
            noexcept(noexcept(0 - t))
            requires requires { 0 - t; }
        {
            return 0 - t;
        };
    };
}