#pragma once

#include "concepts/concepts.h"
#include "type_traits/core_traits.h"

namespace stdsharp
{

    template<typename T>
    class default_post_incremention_and_decremention
    {
        template<::std::same_as<T> U>
        [[nodiscard]] friend constexpr auto operator++(U& t, int) // NOLINT(cert-dcl21-cpp)
            noexcept(concepts::nothrow_copy_constructible<U>&& noexcept(++t)) //
            requires requires
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
            noexcept(concepts::nothrow_copy_constructible<U>&& noexcept(--t)) //
            requires requires
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

    template<typename T, typename Delegate>
    class default_pre_incremention_and_decremention : default_post_incremention_and_decremention<T>
    {
        static constexpr Delegate delegate{};

        friend constexpr T& operator++(::std::same_as<T> auto& t) //
            noexcept(noexcept(++delegate(t))) //
            requires requires
        {
            ++delegate(t);
        }
        {
            ++delegate(t);
            return t;
        }

        friend constexpr T& operator--(::std::same_as<T> auto& t) //
            noexcept(noexcept(--delegate(t))) //
            requires requires
        {
            --delegate(t);
        }
        {
            --delegate(t);
            return t;
        }
    };

    template<
        typename T,
        bool Exchangable = true,
        ::std::constructible_from Delegate = type_traits::empty_t // clang-format off
    > // clang-format on
    class default_arithmetic_operation : default_post_incremention_and_decremention<T>
    {
        static constexpr Delegate delegate{};

#define BS_ARITH_OP(op)                                                                   \
    template<::std::same_as<T> This>                                                      \
    [[nodiscard]] friend constexpr T operator op(                                         \
        const This& t, /**/                                                               \
        const concepts::not_same_as<T> auto& u /* clang-format off */                     \
    ) /* clang-format on */                                                           \
        noexcept(noexcept(This{delegate(t) op u})) /**/                                   \
        requires requires                                                                 \
    {                                                                                     \
        requires ::std::constructible_from<This, decltype(delegate(t) op u)> &&           \
            !::std::same_as<Delegate, type_traits::empty_t>;                              \
    }                                                                                     \
    {                                                                                     \
        return T{delegate(t) op u};                                                       \
    }                                                                                     \
                                                                                          \
    template<::std::same_as<T> This>                                                      \
    [[nodiscard]] friend constexpr T operator op(const This& t, const This& u) /**/       \
        noexcept(noexcept(This{delegate(t) op delegate(u)})) /**/                         \
        requires requires                                                                 \
    {                                                                                     \
        requires ::std::constructible_from<This, decltype(delegate(t) op delegate(u))> && \
            !::std::same_as<Delegate, type_traits::empty_t>;                              \
    }                                                                                     \
    {                                                                                     \
        return T{delegate(t) op delegate(u)};                                             \
    }                                                                                     \
                                                                                          \
    [[nodiscard]] friend constexpr T operator op(                                         \
        T t, /**/                                                                         \
        const concepts::not_same_as<T> auto& u /* clang-format off */                     \
    ) noexcept(noexcept(t op## = u)) /* clang-format on */                            \
        requires requires                                                                 \
    {                                                                                     \
        {                                                                                 \
            t op## = u                                                                    \
            } -> ::std::same_as<T&>;                                                      \
    }                                                                                     \
    {                                                                                     \
        return t op## = u;                                                                \
    }                                                                                     \
                                                                                          \
    template<typename U>                                                                  \
    [[nodiscard]] friend constexpr T operator op(const U& u, const T& t) /**/             \
        noexcept(noexcept(t op u)) /**/                                                   \
        requires requires                                                                 \
    {                                                                                     \
        requires Exchangable && !::std::same_as<T, U>;                                    \
        {                                                                                 \
            t op u                                                                        \
            } -> ::std::same_as<T>;                                                       \
    }                                                                                     \
    {                                                                                     \
        return t op u;                                                                    \
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

    template<typename T, ::std::constructible_from Delegate>
    class default_arithmetic_assign_operation :
        default_pre_incremention_and_decremention<T, Delegate>
    {
        static constexpr Delegate delegate{};

#define BS_ARITH_OP(op)                                                                      \
    template<::std::same_as<T> This>                                                         \
    friend constexpr T& operator op##=(This& t, const concepts::not_same_as<T> auto& u) /**/ \
        noexcept(noexcept(delegate(t) op## = u)) /**/                                        \
        requires requires                                                                    \
    {                                                                                        \
        delegate(t) op## = u;                                                                \
    }                                                                                        \
    {                                                                                        \
        delegate(t) op## = u;                                                                \
        return t;                                                                            \
    }                                                                                        \
                                                                                             \
    template<::std::same_as<T> This>                                                         \
    friend constexpr T& operator op##=(This& t, const This& u) /**/                          \
        noexcept(noexcept(delegate(t) op## = delegate(u))) /**/                              \
        requires requires                                                                    \
    {                                                                                        \
        delegate(t) op## = delegate(u);                                                      \
    }                                                                                        \
    {                                                                                        \
        delegate(t) op## = delegate(u);                                                      \
        return t;                                                                            \
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

#define BS_ARITH_OP(op)                                           \
    template<::std::same_as<T> U>                                 \
    [[nodiscard]] friend constexpr T operator op(const U& t) /**/ \
        noexcept(noexcept(U{op delegate(t)})) /**/                \
        requires requires                                         \
    {                                                             \
        U{op delegate(t)};                                        \
    }                                                             \
    {                                                             \
        return T{op delegate(t)};                                 \
    }

        BS_ARITH_OP(~);
        BS_ARITH_OP(-);

#undef BS_ARITH_OP
    };

}