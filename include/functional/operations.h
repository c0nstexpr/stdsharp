//
// Created by BlurringShadow on 2021-9-18.
//

#pragma once

#include <functional>

#include "type_traits/type_traits.h"
#include "iterator/iterator.h"
#include "functional/invocable_obj.h"

namespace stdsharp::functional
{
    template<typename T>
    inline constexpr invocable_obj constructor(
        nodiscard_tag,
        []<typename... Args> // clang-format off
            requires ::std::constructible_from<T, Args...> // clang-format on
        (Args&&... args) noexcept(concepts::nothrow_constructible_from<T, Args...>) //
        {
            return T{::std::forward<Args>(args)...}; //
        } //
    );

    inline constexpr invocable_obj copy(
        nodiscard_tag, //
        []<typename T, ::std::copy_constructible DecayT = ::std::decay_t<T>>(T&& t) //
        noexcept(concepts::nothrow_copy_constructible<DecayT>)
            ->DecayT { return t; } //
    );

    namespace details
    {
        struct assign
        {
            template<typename T, typename U = T>
                requires ::std::assignable_from<T&, U>
            constexpr decltype(auto) operator()(T& left, U&& right) const
                noexcept(noexcept(left = ::std::forward<U>(right)))
            {
                return left = ::std::forward<U>(right);
            }
        };

        struct assign_by_construct
        {
            template<typename T, typename... U, typename ActualT = ::std::remove_reference_t<T>>
                requires ::std::constructible_from<ActualT, U...>
            constexpr decltype(auto) operator()(T& left, U&&... right) const
                noexcept(noexcept(left = ActualT{::std::forward<U>(right)...}))
            {
                return left = ActualT{::std::forward<U>(right)...};
            }
        };
    }

    inline constexpr struct assign :
        ::ranges::overloaded<details::assign, details::assign_by_construct>,
        nodiscard_tag_t
    {
    } assign_v{};


    inline constexpr ::std ::ranges ::equal_to equal_to_v{};
    inline constexpr ::std ::ranges ::not_equal_to not_equal_to_v{};
    inline constexpr ::std ::ranges ::less less_v{};
    inline constexpr ::std ::ranges ::greater greater_v{};
    inline constexpr ::std ::ranges ::less_equal less_equal_v{};
    inline constexpr ::std ::ranges ::greater_equal greater_equal_v{};
    inline constexpr ::std::compare_three_way compare_three_way_v{};
    inline constexpr ::std ::plus<> plus_v{};
    inline constexpr ::std ::minus<> minus_v{};
    inline constexpr ::std ::divides<> divides_v{};
    inline constexpr ::std ::multiplies<> multiplies_v{};
    inline constexpr ::std ::modulus<> modulus_v{};
    inline constexpr ::std ::negate<> negate_v{};
    inline constexpr ::std ::logical_and<> logical_and_v{};
    inline constexpr ::std ::logical_not<> logical_not_v{};
    inline constexpr ::std ::logical_or<> logical_or_v{};
    inline constexpr ::std ::bit_and<> bit_and_v{};
    inline constexpr ::std ::bit_not<> bit_not_v{};
    inline constexpr ::std ::bit_or<> bit_or_v{};
    inline constexpr ::std ::bit_xor<> bit_xor_v{};


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
    namespace details                                                                             \
    {                                                                                             \
        struct operator_type##_assign                                                             \
        {                                                                                         \
            template<typename T, typename U>                                                      \
            requires operator_type##_assignable_from<T, U> constexpr decltype(auto)               \
                operator()(T& l, U&& u) const noexcept(noexcept((l op## = ::std::forward<U>(u)))) \
            {                                                                                     \
                return l op## = ::std::forward<U>(u);                                             \
            }                                                                                     \
        };                                                                                        \
                                                                                                  \
                                                                                                  \
        struct indirect_##operator_type##_assign                                                  \
        {                                                                                         \
            template<typename T, typename U>                                                      \
                requires requires(T l, U&& u) { l = operator_type##_v(l, ::std::forward<U>(u)); } \
            constexpr decltype(auto) operator()(T& l, U&& u) const                                \
                noexcept(noexcept((l = operator_type##_v(l, ::std::forward<U>(u)))))              \
            {                                                                                     \
                return l = operator_type##_v(l, ::std::forward<U>(u));                            \
            }                                                                                     \
        };                                                                                        \
    }                                                                                             \
                                                                                                  \
    inline constexpr struct operator_type##_assign :                                              \
        ::ranges::overloaded<                                                                     \
            details::operator_type##_assign,                                                      \
            details::indirect_##operator_type##_assign>,                                          \
        nodiscard_tag_t                                                                           \
    {                                                                                             \
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

#define BS_UTIL_ASSIGN_OPERATE(operator_type)                                                 \
    inline constexpr struct operator_type##_assign                                            \
    {                                                                                         \
        template<typename T, typename U>                                                      \
            requires requires(T l, U&& u) { l = operator_type##_v(l, ::std::forward<U>(u)); } \
        constexpr decltype(auto) operator()(T& l, U&& u) const                                \
            noexcept(noexcept((l = operator_type##_v(l, ::std::forward<U>(u)))))              \
        {                                                                                     \
            return l = operator_type##_v(l, ::std::forward<U>(u));                            \
        }                                                                                     \
    } operator_type##_assign_v{};

    BS_UTIL_ASSIGN_OPERATE(negate)
    BS_UTIL_ASSIGN_OPERATE(logical_and)
    BS_UTIL_ASSIGN_OPERATE(logical_not)
    BS_UTIL_ASSIGN_OPERATE(logical_or)

#undef BS_UTIL_ASSIGN_OPERATE

    inline constexpr ::std::identity identity_v{};

#define BS_UTIL_INCREMENT_DECREMENT_OPERATE(operator_prefix, op, al_op)                            \
    inline constexpr struct pre_##operator_prefix##crease                                          \
    {                                                                                              \
        template<::std::weakly_incrementable T>                                                    \
        constexpr decltype(auto) operator()(T& v) const noexcept(noexcept(op##op v))               \
        {                                                                                          \
            return op##op v;                                                                       \
        }                                                                                          \
    } pre_##operator_prefix##crease_v{};                                                           \
                                                                                                   \
    inline constexpr struct post_##operator_prefix##crease                                         \
    {                                                                                              \
        template<iterator::weakly_decrementable T>                                                 \
        [[nodiscard]] constexpr decltype(auto) operator()(T& v) const noexcept(noexcept(v op##op)) \
        {                                                                                          \
            return v op##op;                                                                       \
        }                                                                                          \
    } post_##operator_prefix##crease_v{};

    BS_UTIL_INCREMENT_DECREMENT_OPERATE(in, +, plus)
    BS_UTIL_INCREMENT_DECREMENT_OPERATE(de, -, minus)

#undef BS_UTIL_INCREMENT_DECREMENT_OPERATE

    namespace details
    {
        struct advance_by_op
        {
            template<typename T, concepts::unsigned_ Distance = ::std::iter_difference_t<T>>
                requires ::std::invocable<pre_increase, T>
            constexpr decltype(auto) operator()(T& v, Distance distance) const //
                noexcept(concepts::nothrow_invocable<pre_increase, T>)
            {
                if(distance == 0) return v;
                for(; distance > 0; --distance) pre_increase_v(v);
                return v;
            }

            template<typename T, concepts::signed_ Distance = ::std::iter_difference_t<T>>
                requires( //
                    ::std::invocable<advance_by_op, T, ::std::make_unsigned_t<Distance>>&& //
                    ::std::invocable<pre_decrease, T> //
                )
            constexpr decltype(auto) operator()(T& v, Distance distance) const noexcept( //
                noexcept( //
                    concepts::
                        nothrow_invocable<advance_by_op, T, ::std::make_unsigned_t<Distance>>&&
                            concepts::nothrow_invocable<pre_decrease, T> // clang-format off
                ) // clang-format on
            )
            {
                if(distance >= 0) return (*this)(v, type_traits::make_unsigned(distance));

                for(; distance < 0; ++distance) pre_decrease_v(v);
                return v;
            }
        };
    }

    inline constexpr struct advance : ::ranges::overloaded<plus_assign, details::advance_by_op>
    {
    } advance_v{};
}
