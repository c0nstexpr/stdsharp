#pragma once

#include "concepts/object.h"

namespace stdsharp::default_operator
{
    struct increase
    {
        template<std::copy_constructible T>
        [[nodiscard]] constexpr auto operator++(this T& t, int)
            noexcept(nothrow_copy_constructible<T> && noexcept(++t))
            requires requires { ++t; }
        {
            auto copied = t;
            ++t;
            return cpp_move(copied);
        }

        template<std::copy_constructible T>
        [[nodiscard]] constexpr auto operator--(this T& t, int)
            noexcept(nothrow_copy_constructible<T> && noexcept(--t))
            requires requires { --t; }
        {
            auto copied = t;
            --t;
            return cpp_move(copied);
        }
    };

    struct arithmetic
    {
        constexpr auto& operator++(this auto& t) noexcept(noexcept(t += 1))
            requires requires { t += 1; }
        {
            t += 1;
            return t;
        }

        constexpr auto& operator--(this auto& t) noexcept(noexcept(t -= 1))
            requires requires { t -= 1; }
        {
            t -= 1;
            return t;
        }

#define STDSHARP_ARITH_OP(op)                                                        \
    template<std::move_constructible T>                                              \
    [[nodiscard]] constexpr T operator op(this T t, auto&& u) /**/                   \
        noexcept(noexcept(t op## = cpp_forward(u)) && nothrow_move_constructible<T>) \
        requires requires { t op## = cpp_forward(u); }                               \
    {                                                                                \
        t op## = cpp_forward(u);                                                     \
        return cpp_move(t);                                                          \
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

        [[nodiscard]] constexpr auto operator-(this const auto& t)
            noexcept(noexcept(0 - t)) -> decltype(0 - t)
        {
            return 0 - t;
        };
    };

    struct arrow
    {
        [[nodiscard]] constexpr decltype(auto) operator->(this dereferenceable auto&& t)
            noexcept(noexcept(*cpp_forward(t)))
        {
            return *cpp_forward(t);
        }

        [[nodiscard]] constexpr decltype(auto
        ) operator->*(this dereferenceable auto&& t, auto&& ptr)
            noexcept(noexcept((*cpp_forward(t)).*cpp_forward(ptr)))
        {
            return (*cpp_forward(t)).*cpp_forward(ptr);
        }
    };

    struct subscript
    {
#if __cpp_multidimensional_subscript >= 202110L
        [[nodiscard]] constexpr decltype(auto
        ) operator[](this auto&& t, auto&& first_arg, auto&&... args) noexcept( //
            noexcept(cpp_forward(t)[cpp_forward(first_arg)][cpp_forward(args)...])
        )
            requires requires { cpp_forward(t)[cpp_forward(first_arg)][cpp_forward(args)...]; }
        {
            return cpp_forward(t)[cpp_forward(first_arg)][cpp_forward(args)...];
        }
#endif
    };
}