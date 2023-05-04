#pragma once

#include "concepts/concepts.h"
#include "type_traits/core_traits.h"

namespace stdsharp
{

    template<typename T>
    class default_post_increase_and_decrease
    {
        template<::std::same_as<T> U>
        [[nodiscard]] friend constexpr auto operator++(U& t, int) // NOLINT(cert-dcl21-cpp)
            noexcept(nothrow_copy_constructible<U>&& noexcept(++t))
            requires requires //
        {
            requires ::std::copy_constructible<U>;
            ++t;
        }
        {
            const auto copy = t;
            ++t;
            return copy;
        }

        template<::std::same_as<T> U>
        [[nodiscard]] friend constexpr auto operator--(U& t, int) // NOLINT(cert-dcl21-cpp)
            noexcept(nothrow_copy_constructible<U>&& noexcept(--t))
            requires requires //
        {
            requires ::std::copy_constructible<U>;
            --t;
        }
        {
            const auto copy = t;
            --t;
            return copy;
        }
    };

    template<typename T, ::std::constructible_from Delegate = ::std::identity>
        requires requires { constant<(true, Delegate{})>{}; }
    class default_pre_increase_and_decrease : default_post_increase_and_decrease<T>
    {
        static constexpr Delegate delegate{};

        friend constexpr T& operator++(::std::same_as<T> auto& t) noexcept(noexcept(++delegate(t)))
            requires requires { ++delegate(t); }
        {
            ++delegate(t);
            return t;
        }

        friend constexpr T& operator--(::std::same_as<T> auto& t) //
            noexcept(noexcept(--delegate(t))) //
            requires requires { --delegate(t); }
        {
            --delegate(t);
            return t;
        }
    };

    template<typename T, bool Commutable = true>
    class default_arithmetic_operation : default_post_increase_and_decrease<T>
    {
#define BS_ARITH_OP(op)                                                                          \
    template<typename U>                                                                         \
    [[nodiscard]] friend constexpr T operator op(T t, const U& u) noexcept(noexcept(t op## = u)) \
        requires requires {                                                                      \
            {                                                                                    \
                t op## = u                                                                       \
            } -> ::std::same_as<T&>;                                                             \
        }                                                                                        \
    {                                                                                            \
        return cpp_move(t op## = u);                                                             \
    }                                                                                            \
                                                                                                 \
    template<not_same_as<T> U, decay_same_as<T> This>                                            \
        requires Commutable && requires(const U& u, T t) { t op u; }                             \
    [[nodiscard]] friend constexpr T operator op(const U& u, This&& t) noexcept(                 \
        noexcept(cpp_forward(t) op u)                                                            \
    )                                                                                            \
    {                                                                                            \
        return cpp_forward(t) op u;                                                              \
    }

        BS_ARITH_OP(+)
        BS_ARITH_OP(-)
        BS_ARITH_OP(*)
        BS_ARITH_OP(/)
        BS_ARITH_OP(%)
        BS_ARITH_OP(&)
        BS_ARITH_OP(|)
        BS_ARITH_OP(^)
        BS_ARITH_OP(<<)
        BS_ARITH_OP(>>)

#undef BS_ARITH_OP

        [[nodiscard]] friend constexpr T& operator+(::std::same_as<T> auto& t) noexcept
        {
            return t;
        }

        [[nodiscard]] friend constexpr T&& operator+(::std::same_as<T> auto&& t) noexcept
        {
            return static_cast<T&&>(t);
        }
    };

    template<typename T, ::std::constructible_from Delegate = ::std::identity>
        requires requires { constant<(true, Delegate{})>{}; }
    class default_arithmetic_assign_operation : default_pre_increase_and_decrease<T, Delegate>
    {
        static constexpr Delegate delegate{};

#define BS_ARITH_OP(op)                                                                 \
    template<::std::same_as<T> This>                                                    \
    friend constexpr T& operator op##=(This& t, const not_same_as<T> auto& u) noexcept( \
        noexcept(delegate(t) op## = u)                                                  \
    )                                                                                   \
        requires requires { delegate(t) op## = u; }                                     \
    {                                                                                   \
        delegate(t) op## = u;                                                           \
        return t;                                                                       \
    }                                                                                   \
                                                                                        \
    template<::std::same_as<T> This>                                                    \
    friend constexpr T& operator op##=(This& t, const This& u) noexcept(                \
        noexcept(delegate(t) op## = delegate(u))                                        \
    )                                                                                   \
        requires requires { delegate(t) op## = delegate(u); }                           \
    {                                                                                   \
        delegate(t) op## = delegate(u);                                                 \
        return t;                                                                       \
    }

        BS_ARITH_OP(+)
        BS_ARITH_OP(-)
        BS_ARITH_OP(*)
        BS_ARITH_OP(/)
        BS_ARITH_OP(%)
        BS_ARITH_OP(&)
        BS_ARITH_OP(|)
        BS_ARITH_OP(^)
        BS_ARITH_OP(<<)
        BS_ARITH_OP(>>)

#undef BS_ARITH_OP

#define BS_ARITH_OP(op)                                                                            \
    template<::std::same_as<T> U>                                                                  \
    [[nodiscard]] friend constexpr T operator op(const U& t) noexcept(noexcept(U{op delegate(t)})) \
        requires requires { U{op delegate(t)}; }                                                   \
    {                                                                                              \
        return T{op delegate(t)};                                                                  \
    }

        BS_ARITH_OP(~);
        BS_ARITH_OP(-);

#undef BS_ARITH_OP
    };
}