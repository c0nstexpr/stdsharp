//
// Created by BlurringShadow on 2021-9-18.
//

#pragma once

#include <functional>
#include "functional/invocable_obj.h"

namespace stdsharp::functional
{
    template<typename T>
    inline constexpr ::stdsharp::functional::invocable_obj constructor(
        ::stdsharp::functional::nodiscard_tag,
        []<typename... Args>(Args&&... args) // clang-format off
            noexcept(::stdsharp::concepts::nothrow_constructible_from<T, Args...>)
            -> ::std::enable_if_t<::std::constructible_from<T, Args...>, T> // clang-format on
        {
            return T{::std::forward<Args>(args)...}; //
        } //
    );

    inline constexpr struct assign
    {
        template<typename T, typename U>
            requires ::std::assignable_from<T, U>
        constexpr decltype(auto) operator()(T& left, U&& right) const
            noexcept(noexcept(left = ::std::forward<U>(right)))
        {
            return left = ::std::forward<U>(right);
        }

        template<typename T, typename... U>
            requires ::std::constructible_from<::std::remove_cvref_t<T>, U...>
        constexpr decltype(auto) operator()(T& left, U&&... right) const
            noexcept(noexcept(left = ::std::remove_cvref_t<T>{::std::forward<U>(right)...}))
        {
            return left = ::std::remove_cvref_t<T>{::std::forward<U>(right)...};
        }
    } assign_v{};

    inline constexpr ::std::ranges::equal_to equal_to_v{};
    inline constexpr ::std::ranges::not_equal_to not_equal_to_v{};
    inline constexpr ::std::ranges::less less_v{};
    inline constexpr ::std::ranges::greater greater_v{};
    inline constexpr ::std::ranges::less_equal less_equal_v{};
    inline constexpr ::std::ranges::greater_equal greater_equal_v{};
    inline constexpr ::std::compare_three_way compare_three_way_v{};

    inline constexpr ::std::plus<> plus_v{};
    inline constexpr ::std::minus<> minus_v{};
    inline constexpr ::std::divides<> divides_v{};
    inline constexpr ::std::multiplies<> multiplies_v{};
    inline constexpr ::std::modulus<> modulus_v{};
    inline constexpr ::std::negate<> negate_v{};

    inline constexpr ::std::logical_and<> logical_and_v{};
    inline constexpr ::std::logical_not<> logical_not_v{};
    inline constexpr ::std::logical_or<> logical_or_v{};

    inline constexpr ::std::bit_and<> bit_and_v{};
    inline constexpr ::std::bit_not<> bit_not_v{};
    inline constexpr ::std::bit_or<> bit_or_v{};
    inline constexpr ::std::bit_xor<> bit_xor_v{};

#define BS_UTIL_SHIFT_OPERATE(direction, operate)                                        \
    inline constexpr struct direction##_shift                                            \
    {                                                                                    \
        template<typename T, typename U>                                                 \
            requires requires(T && left, U&& right)                                      \
            {                                                                            \
                ::std::forward<T>(left) operate ::std::forward<U>(right);                \
            }                                                                            \
        [[nodiscard]] constexpr decltype(auto) operator()(T&& left, U&& right) const     \
            noexcept(noexcept(::std::forward<T>(left) operate ::std::forward<U>(right))) \
        {                                                                                \
            return ::std::forward<T>(left) operate ::std::forward<U>(right);             \
        }                                                                                \
    } direction##_shift_v{};

    BS_UTIL_SHIFT_OPERATE(left, <<)
    BS_UTIL_SHIFT_OPERATE(right, >>)

#undef BS_UTIL_SHIFT_OPERATE

#define BS_UTIL_ASSIGN_OPERATE(operator_type, op)                                                 \
                                                                                                  \
    template<typename T, typename U>                                                              \
    concept operator_type##_assignable_from = requires(T l, U && u)                               \
    {                                                                                             \
        l op## = ::std::forward<U>(u);                                                            \
    };                                                                                            \
                                                                                                  \
    inline constexpr struct operator_type##_assign                                                \
    {                                                                                             \
        template<typename T, typename U>                                                          \
        requires ::stdsharp::functional::                                                         \
            operator_type##_assignable_from<T, U> constexpr decltype(auto)                        \
                operator()(T& l, U&& u) const noexcept(noexcept((l op## = ::std::forward<U>(u)))) \
        {                                                                                         \
            return l op## = ::std::forward<U>(u);                                                 \
        }                                                                                         \
                                                                                                  \
        template<typename T, typename U>                                                          \
            requires requires(T l, U&& u)                                                         \
            {                                                                                     \
                !::stdsharp::functional::operator_type##_assignable_from<T, U>;                   \
                l = operator_type##_v(l, ::std::forward<U>(u));                                   \
            }                                                                                     \
        constexpr decltype(auto) operator()(T& l, U&& u) const                                    \
            noexcept(noexcept((l = operator_type##_v(l, ::std::forward<U>(u)))))                  \
        {                                                                                         \
            return l = operator_type##_v(l, ::std::forward<U>(u));                                \
        }                                                                                         \
    } operator_type##_assign_v{};

    BS_UTIL_ASSIGN_OPERATE(plus, +)
    BS_UTIL_ASSIGN_OPERATE(minus, -)
    BS_UTIL_ASSIGN_OPERATE(divides, /)
    BS_UTIL_ASSIGN_OPERATE(multiplies, *)
    BS_UTIL_ASSIGN_OPERATE(modulus, %)
    BS_UTIL_ASSIGN_OPERATE(bit_and, &)
    BS_UTIL_ASSIGN_OPERATE(bit_or, |)
    BS_UTIL_ASSIGN_OPERATE(left_shift, <<)
    BS_UTIL_ASSIGN_OPERATE(right_shift, >>)

#undef BS_UTIL_ASSIGN_OPERATE

    inline constexpr ::std::identity identity_v{};

#define BS_UTIL_INCREMENT_DECREMENT_OPERATE(operator_prefix, op, al_op)                         \
    inline constexpr struct pre_##operator_prefix##crease                                       \
    {                                                                                           \
        template<typename T>                                                                    \
            requires ::std::invocable<al_op##_assign, T, ::std::size_t>                         \
        constexpr decltype(auto) operator()(T& v, const ::std::size_t i = 1) const              \
            noexcept(::stdsharp::concepts::/**/                                                 \
                     nothrow_invocable<al_op##_assign, T, const ::std::size_t>)                 \
        {                                                                                       \
            return al_op##_assign_v(v, i);                                                      \
        }                                                                                       \
                                                                                                \
        template<typename T>                                                                    \
            requires requires(T v)                                                              \
            {                                                                                   \
                !::std::invocable<al_op##_assign, T, ::std::size_t>;                            \
                op##op v;                                                                       \
            }                                                                                   \
        constexpr decltype(auto) operator()(T& v, ::std::size_t i = 1) const                    \
            noexcept(noexcept(op##op v))                                                        \
        {                                                                                       \
            for(; i > 0; --i) op##op v;                                                         \
            return v;                                                                           \
        }                                                                                       \
    } pre_##operator_prefix##crease_v{};                                                        \
                                                                                                \
    inline constexpr struct post_##operator_prefix##crease                                      \
    {                                                                                           \
        template<typename T>                                                                    \
            requires ::std::invocable<::stdsharp::functional::al_op##_assign, T, ::std::size_t> \
        [[nodiscard]] constexpr auto operator()(T& v, const ::std::size_t i = 1) const          \
            noexcept(/**/                                                                       \
                     ::stdsharp::concepts::nothrow_invocable<                                   \
                         ::stdsharp::functional::al_op##_assign,                                \
                         T&,                                                                    \
                         const ::std::size_t> /**/                                              \
            )                                                                                   \
        {                                                                                       \
            const auto old = v;                                                                 \
            ::stdsharp::functional::al_op##_assign_v(v, i);                                     \
            return old;                                                                         \
        }                                                                                       \
                                                                                                \
        template<typename T>                                                                    \
            requires requires(T v)                                                              \
            {                                                                                   \
                !::std::invocable<al_op##_assign, T, ::std::size_t>;                            \
                op##op v;                                                                       \
                v op##op;                                                                       \
            }                                                                                   \
        [[nodiscard]] constexpr auto operator()(T& v, ::std::size_t i = 1) const                \
            noexcept(noexcept(v op##op) && noexcept(op##op v))                                  \
        {                                                                                       \
            if(i == 0) return v op##op;                                                         \
                                                                                                \
            const auto old = v;                                                                 \
            for(; i > 0; --i) op##op v;                                                         \
            return old;                                                                         \
        }                                                                                       \
    } post_##operator_prefix##crease_v{};

    BS_UTIL_INCREMENT_DECREMENT_OPERATE(in, +, plus)
    BS_UTIL_INCREMENT_DECREMENT_OPERATE(de, -, minus)

#undef BS_UTIL_INCREMENT_DECREMENT_OPERATE

    inline constexpr struct advance
    {
        template<typename T, typename Distance>
            requires ::std::invocable<plus_assign, T, Distance>
        constexpr decltype(auto) operator()(T& v, Distance&& distance) const
            noexcept(::stdsharp::concepts::nothrow_invocable<plus_assign, T&, Distance>)
        {
            return ::stdsharp::functional::plus_assign_v(v, ::std::forward<Distance>(distance));
        }

        template<typename T, typename Distance>
            requires requires
            {
                !::std::invocable<plus_assign, T, Distance>&& //
                    ::std::invocable<::stdsharp::functional::pre_increase, T, Distance>&& //
                    ::std::invocable<::stdsharp::functional::pre_decrease, T, Distance>;
            }
        constexpr decltype(auto) operator()(T& v, const Distance& distance) const //
            noexcept( //
                noexcept(
                    ::stdsharp::functional::pre_increase_v(v, distance),
                    ::stdsharp::functional::pre_decrease_v(v, distance) // clang-format off
                ) // clang-format on
            )
        {
            return distance > 0 ? //
                ::stdsharp::functional::pre_increase_v(v, distance) :
                ::stdsharp::functional::pre_decrease_v(v, -distance);
        }
    } advance_v{};
}
