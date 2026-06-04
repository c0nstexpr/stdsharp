#pragma once

#include "concepts/object.h"
#include "functional/sequenced_invocables.h"

namespace stdsharp::default_operator
{
    struct increase
    {
        template<std::copy_constructible T>
        [[nodiscard]] constexpr auto
            operator++(this T& t, int) noexcept(nothrow_copy_constructible<T> && noexcept(++t))
            requires requires { ++t; }
        {
            auto copied = t;
            ++t;
            return copied;
        }

        template<std::copy_constructible T>
        [[nodiscard]] constexpr auto
            operator--(this T& t, int) noexcept(nothrow_copy_constructible<T> && noexcept(--t))
            requires requires { --t; }
        {
            auto copied = t;
            --t;
            return copied;
        }
    };

    struct arithmetic : increase
    {
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

    struct unary_plus
    {
        [[nodiscard]] constexpr decltype(auto) operator+(this auto&& t) noexcept
        {
            return cpp_forward(t);
        }
    };

#define STDSHARP_ARITH_OP(name, op)                                 \
    struct name##_commutative                                       \
    {                                                               \
        template<not_decay_derived<name##_commutative> T>           \
        [[nodiscard]] friend constexpr decltype(auto) operator op(  \
            T&& u,                                                  \
            decay_derived<name##_commutative> auto&& t              \
        ) noexcept(noexcept(cpp_forward(t) op cpp_forward(u)))      \
            requires requires { cpp_forward(t) op cpp_forward(u); } \
        {                                                           \
            return cpp_forward(t) op cpp_forward(u);                \
        }                                                           \
    }

    STDSHARP_ARITH_OP(plus, +);
    STDSHARP_ARITH_OP(minus, -);
    STDSHARP_ARITH_OP(multiply, *);
    STDSHARP_ARITH_OP(divide, /);
    STDSHARP_ARITH_OP(modulus, %);
    STDSHARP_ARITH_OP(bitwise_and, &);
    STDSHARP_ARITH_OP(bitwise_or, |);
    STDSHARP_ARITH_OP(bitwise_xor, ^);
    STDSHARP_ARITH_OP(bitwise_left_shift, <<);
    STDSHARP_ARITH_OP(bitwise_right_shift, >>);

#undef STDSHARP_ARITH_OP

    struct subscript
    {
    private:
        template<typename Self>
        static constexpr auto self_cast = forward_like<Self, subscript>;

        struct subscript_direct
        {
            template<typename Self>
            [[nodiscard]] static constexpr decltype(auto) operator()(
                Self&& t,
                auto&& first_arg,
                auto&&... args //
            ) noexcept(noexcept(cpp_forward(t)[cpp_forward(first_arg)][cpp_forward(args)...]))
                requires requires {
                    requires sizeof...(args) > 0;
                    cpp_forward(t)[cpp_forward(first_arg)][cpp_forward(args)...];
                }
            {
                return cpp_forward(t)[cpp_forward(first_arg)][cpp_forward(args)...];
            }
        };

        template<typename T, typename... Args>
        struct i{};

        struct subscript_recursive
        {
            template<
                typename Self,
                typename FirstArg,
                typename... Args,
                typename First = decltype(std::declval<Self>()[std::declval<FirstArg>()])>
                requires(sizeof...(Args) > 0) && std::invocable<subscript_recursive, First, Args...>
            [[nodiscard]] static constexpr decltype(auto) operator()(
                Self&& self,
                FirstArg&& first_arg,
                Args&&... args //
            ) noexcept(nothrow_invocable<subscript_recursive, First, Args...>)
            {
                return operator()(cpp_forward(self)[cpp_forward(first_arg)], cpp_forward(args)...);
            }
        };

        using subscript_impl = sequenced_invocables<subscript_direct, subscript_recursive>;

    public:
        template<typename Self, typename... Args>
            requires std::invocable<subscript_impl, Self, Args...>
        [[nodiscard]] constexpr decltype(auto) operator[](this Self&& t, Args&&... args) //
            noexcept(nothrow_invocable<subscript_impl, Self, Args...>)
        {
            return subscript_impl{}(self_cast<Self>(t), cpp_forward(args)...);
        }
    };

    struct arrow
    {
        template<typename T>
        [[nodiscard]] constexpr auto*
            operator->(this T&& t) noexcept(noexcept(std::addressof(*cpp_forward(t))))
            requires requires { std::addressof(*cpp_forward(t)); }
        {
            return std::addressof(*cpp_forward(t));
        }

        template<typename T>
        [[nodiscard]] constexpr auto operator->*(this T&& t, auto&& ptr) noexcept(
            noexcept((*cpp_forward(t)).*(cpp_forward(ptr)))
        )
            requires requires { (*cpp_forward(t)).*(cpp_forward(ptr)); }
        {
            return (*cpp_forward(t)).*(cpp_forward(ptr));
        }
    };
}
