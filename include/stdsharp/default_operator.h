#pragma once

#include "concepts/object.h"

namespace stdsharp::default_operator
{
    struct increase
    {
        template<std::copy_constructible T>
        [[nodiscard]] friend constexpr auto operator++(T& t, int)
            noexcept(nothrow_copy_constructible<T> && noexcept(++t))
            requires requires { ++t; }
        {
            auto copied = t;
            ++t;
            return cpp_move(copied);
        }

        template<std::copy_constructible T>
        [[nodiscard]] friend constexpr auto operator--(T& t, int)
            noexcept(nothrow_copy_constructible<T> && noexcept(--t))
            requires requires { --t; }
        {
            auto copied = t;
            --t;
            return cpp_move(copied);
        }
    };

    struct arithmetic : increase
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

#undef STDSHARP_ARITH_OP
    };

    struct inverse_arithmetic : arithmetic
    {
#define STDSHARP_ARITH_OP(op)                                                            \
    template<typename U>                                                                 \
    [[nodiscard]] friend constexpr decltype(auto) operator op(U&& u, auto&& t) /**/      \
        noexcept(noexcept(cpp_forward(t) op cpp_forward(u)))                             \
        requires requires {                                                              \
            requires !std::is_base_of_v<inverse_arithmetic, std::remove_reference_t<U>>; \
            cpp_forward(t) op cpp_forward(u);                                            \
        }                                                                                \
    {                                                                                    \
        return cpp_forward(t) op cpp_forward(u);                                         \
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

#define STDSHARP_ARITH_OP(op)                                                                 \
    [[nodiscard]] friend constexpr auto operator op(const auto& t) noexcept(noexcept(0 op t)) \
        ->decltype(0 op t)                                                                    \
    {                                                                                         \
        return 0 op t;                                                                        \
    };                                                                                        \
                                                                                              \
    template<std::default_initializable T>                                                    \
    [[nodiscard]] friend constexpr decltype(auto) operator op(const T& t                      \
    ) noexcept(noexcept(T {} op t))                                                           \
        requires requires {                                                                   \
            requires !requires { 0 op t; };                                                   \
            T {}                                                                              \
            op t;                                                                             \
        }                                                                                     \
    {                                                                                         \
        return T {}                                                                           \
        op t;                                                                                 \
    };

        STDSHARP_ARITH_OP(+)
        STDSHARP_ARITH_OP(-)

#undef STDSHARP_ARITH_OP
    };

    struct arrow
    {
        [[nodiscard]] constexpr auto* operator->(this dereferenceable auto&& t)
            noexcept(noexcept(std::addressof(*cpp_forward(t))))
        {
            return std::addressof(*cpp_forward(t));
        }

        [[nodiscard]] constexpr auto operator->*(this dereferenceable auto&& t, auto&& ptr)
            noexcept(noexcept((*cpp_forward(t)).*cpp_forward(ptr)))
            requires requires { (*cpp_forward(t)).*cpp_forward(ptr); }
        {
            return (*cpp_forward(t)).*cpp_forward(ptr);
        }
    };

    struct subscript
    {
#if __cpp_multidimensional_subscript >= 202110L
        [[nodiscard]] constexpr decltype(auto) operator[]( //
            this auto&& t,
            auto&& first_arg,
            auto&& second_arg
        ) noexcept( //
            noexcept( //
                cpp_forward(t) //
                    [cpp_forward(first_arg)] //
                    [cpp_forward(second_arg)]
            )
        )
            requires requires {
                cpp_forward(t) //
                    [cpp_forward(first_arg)] //
                    [cpp_forward(second_arg)];
            }
        {
            return cpp_forward(t) //
                [cpp_forward(first_arg)] //
                [cpp_forward(second_arg)];
        }

        [[nodiscard]] constexpr decltype(auto) operator[]( //
            this auto&& t,
            auto&& first_arg,
            auto&& second_arg,
            auto&&... args
        ) noexcept( //
            noexcept( //
                cpp_forward(t) //
                    [cpp_forward(first_arg)] //
                    [cpp_forward(second_arg)] //
                    [cpp_forward(args)...]
            )
        )
            requires requires {
                cpp_forward(t) //
                    [cpp_forward(first_arg)] //
                    [cpp_forward(second_arg)] //
                    [cpp_forward(args)...];
            }
        {
            return cpp_forward(t) //
                [cpp_forward(first_arg)] //
                [cpp_forward(second_arg)] //
                [cpp_forward(args)...];
        }
#endif
    };
}