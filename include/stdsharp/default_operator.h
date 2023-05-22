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

    template<typename T>
    class default_post_increase_and_decrease : default_unary_op<T>
    {
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

    template<typename T, typename Delegate = ::std::identity>
        requires cpp_is_constexpr(Delegate{})
    class default_pre_increase_and_decrease : default_post_increase_and_decrease<T>
    {
        static constexpr Delegate delegate{};

        friend constexpr T& operator++(T& t) noexcept(noexcept(++delegate(t)))
            requires requires { ++delegate(t); }
        {
            ++delegate(t);
            return t;
        }

        friend constexpr T& operator--(T& t) //
            noexcept(noexcept(--delegate(t))) //
            requires requires { --delegate(t); }
        {
            --delegate(t);
            return t;
        }
    };

    template<typename T, typename Delegate = ::std::identity>
        requires cpp_is_constexpr(Delegate{})
    class default_arithmetic_assign_operation : default_pre_increase_and_decrease<T, Delegate>
    {
        static constexpr Delegate delegate{};

#define STDSHARP_ARITH_OP(op)                                         \
    friend constexpr T& operator op##=(T& t, const auto& u) noexcept( \
        noexcept(delegate(t) op## = u)                                \
    )                                                                 \
        requires requires { delegate(t) op## = u; }                   \
    {                                                                 \
        delegate(t) op## = u;                                         \
        return t;                                                     \
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

#define STDSHARP_ARITH_OP(op)                                                                      \
    [[nodiscard]] friend constexpr T operator op(const T& t) noexcept(noexcept(T{op delegate(t)})) \
        requires requires { T{op delegate(t)}; }                                                   \
    {                                                                                              \
        return T{op delegate(t)};                                                                  \
    }

        STDSHARP_ARITH_OP(~);
        STDSHARP_ARITH_OP(-);

#undef STDSHARP_ARITH_OP
    };

    template<typename T, typename Delegate = ::std::identity, bool Commutable = true>
    class default_arithmetic_operation : default_arithmetic_assign_operation<T, Delegate>
    {
#define STDSHARP_ARITH_OP(op)                                                             \
    [[nodiscard]] friend constexpr T operator op(T t, auto&& u) noexcept(                 \
        noexcept(t op## = cpp_forward(u))                                                 \
    )                                                                                     \
        requires requires { t op## = cpp_forward(u); }                                    \
    {                                                                                     \
        return t op## = cpp_forward(u);                                                   \
    }                                                                                     \
                                                                                          \
    template<typename U>                                                                  \
    [[nodiscard]] friend constexpr T operator op(U&& u, T t) noexcept(noexcept(cpp_move(t \
    ) op cpp_forward(u)))                                                                 \
        requires(!::std::convertible_to<U, T>) && Commutable &&                           \
        requires { cpp_move(t) op cpp_forward(u); }                                       \
    {                                                                                     \
        return cpp_move(t) op cpp_forward(u);                                             \
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
    };
}