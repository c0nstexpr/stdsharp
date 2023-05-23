#pragma once

#include "utility/cast_to.h"
#include "type_traits/core_traits.h"

namespace stdsharp
{
    template<typename T>
    class default_unary_op
    {
        [[nodiscard]] friend constexpr T operator+(const T& t) //
            noexcept(nothrow_copy_constructible<T>)
            requires ::std::copy_constructible<T>
        {
            return t;
        }
    };

    template<typename T, typename Delegate = void>
        requires cpp_is_constexpr(Delegate{})
    class default_increase_and_decrease : default_unary_op<T>
    {
        friend constexpr T& operator++(T& t) noexcept(noexcept(++Delegate{}(t)))
            requires requires { ++Delegate{}(t); }
        {
            ++Delegate{}(t);
            return t;
        }

        friend constexpr T& operator--(T& t) //
            noexcept(noexcept(--Delegate{}(t))) //
            requires requires { --Delegate{}(t); }
        {
            --Delegate{}(t);
            return t;
        }

        [[nodiscard]] friend constexpr auto operator++(T& t, int) //
            noexcept(nothrow_copy_constructible<T>&& noexcept(++t))
            requires ::std::copy_constructible<T> && requires { ++t; }
        {
            const auto copy = t;
            ++t;
            return copy;
        }

        [[nodiscard]] friend constexpr auto operator--(T& t, int) //
            noexcept(nothrow_copy_constructible<T>&& noexcept(--t))
            requires ::std::copy_constructible<T> && requires { --t; }
        {
            const auto copy = t;
            --t;
            return copy;
        }
    };

    template<typename T, typename Delegate = void, bool Commutable = true>
    class default_arithmetic_operation : default_increase_and_decrease<T, Delegate>
    {
#define STDSHARP_ARITH_OP(op)                                             \
    friend constexpr T& operator op##=(T& t, const T& u) noexcept(        \
        noexcept(Delegate{}(t)op## = Delegate{}(u))                       \
    )                                                                     \
        requires requires { Delegate{}(t)op## = Delegate{}(u); }          \
    {                                                                     \
        Delegate{}(t)op## = Delegate{}(u);                                \
        return t;                                                         \
    }                                                                     \
                                                                          \
    friend constexpr T& operator op##=(T& t, const auto& u) noexcept(     \
        noexcept(Delegate{}(t)op## = u)                                   \
    )                                                                     \
        requires requires { Delegate{}(t)op## = u; }                      \
    {                                                                     \
        Delegate{}(t)op## = u;                                            \
        return t;                                                         \
    }                                                                     \
                                                                          \
    [[nodiscard]] friend constexpr T operator op(T t, auto&& u) noexcept( \
        noexcept(t op## = cpp_forward(u))                                 \
    )                                                                     \
        requires requires { t op## = cpp_forward(u); }                    \
    {                                                                     \
        t op## = cpp_forward(u);                                          \
        return t;                                                         \
    }                                                                     \
                                                                          \
    template<typename U, typename V>                                      \
    [[nodiscard]] friend constexpr T operator op(U&& u, V&& t) noexcept(  \
        noexcept(cast_fwd<T>(cpp_forward(t)) op cpp_forward(u))           \
    )                                                                     \
        requires(!::std::convertible_to<U, T>) && Commutable &&           \
        requires { cast_fwd<T>(cpp_forward(t)) op cpp_forward(u); }       \
    {                                                                     \
        return cast_fwd<T>(cpp_forward(t)) op cpp_forward(u);             \
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

#undef STDSHARP_ARITH_OP

#define STDSHARP_ARITH_OP(op)                               \
    [[nodiscard]] friend constexpr T operator op(const T& t \
    ) noexcept(noexcept(T{op Delegate{}(t)}))               \
        requires requires { T{op Delegate{}(t)}; }          \
    {                                                       \
        return T{op Delegate{}(t)};                         \
    }

        STDSHARP_ARITH_OP(~);
        STDSHARP_ARITH_OP(-);

#undef STDSHARP_ARITH_OP
    };
}