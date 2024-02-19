#pragma once

#include "stdsharp/concepts/concepts.h"
#include "type_traits/core_traits.h"

namespace stdsharp
{
    template<typename T>
    class default_increase
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
    class default_arithmetic_operator : default_increase<T>
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

    template<typename T>
    class default_arrow_operator
    {
        [[nodiscard]] constexpr decltype(auto) operator->() const
            noexcept(noexcept(*static_cast<const T&>(*this)))
            requires dereferenceable<const T>
        {
            return *static_cast<const T&>(*this);
        }

        [[nodiscard]] constexpr decltype(auto) operator->() //
            noexcept(noexcept(*static_cast<T&>(*this)))
            requires dereferenceable<T>
        {
            return *static_cast<T&>(*this);
        }

        [[nodiscard]] friend constexpr decltype(auto) operator->*(const T & t, auto&& ptr) //
            noexcept(noexcept((*t).*cpp_forward(ptr)))
            requires dereferenceable<const T>
        {
            return (*t).*cpp_forward(ptr);
        }

        [[nodiscard]] friend constexpr decltype(auto) operator->*(T & t, auto&& ptr) //
            noexcept(noexcept((*t).*cpp_forward(ptr)))
            requires dereferenceable<T>
        {
            return (*t).*cpp_forward(ptr);
        }
    };

    template<typename T>
    class default_subscriptor
    {
#if __cpp_multidimensional_subscript >= 202110L
        [[nodiscard]] constexpr decltype(auto) operator[](auto&& first_arg, auto&&... args) //
            noexcept( //
                noexcept(static_cast<const T&>(*this)[cpp_forward(first_arg)][cpp_forward(args)...])
            )
            requires requires {
                static_cast<const T&>(*this)[cpp_forward(first_arg)][cpp_forward(args)...];
            }
        {
            return static_cast<const T&>(*this)[cpp_forward(first_arg)][cpp_forward(args)...];
        }
#endif
    };
}