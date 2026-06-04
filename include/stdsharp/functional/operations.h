#pragma once

#include "../cstdint/cstdint.h"
#include "../iterator/iterator.h"
#include "compose.h"
#include "sequenced_invocables.h"

namespace stdsharp
{
    inline constexpr struct copy_fn
    {
        template<typename T>
            requires std::constructible_from<std::decay_t<T>, T>
        [[nodiscard]] constexpr std::decay_t<T> operator()(T&& t) const //
            noexcept(nothrow_constructible_from<std::decay_t<T>>)
        {
            return t;
        }
    } copy{};

    inline constexpr struct not_fn
    {
        template<typename Fn>
        [[nodiscard]] constexpr std::decay_t<Fn> operator()(Fn&& fn) const //
            noexcept(noexcept(std::not_fn(cpp_forward(fn))))
            requires requires { std::not_fn(cpp_forward(fn)); }
        {
            return std::not_fn(cpp_forward(fn));
        }
    } not_fn_v{};
}

namespace stdsharp::details
{
    struct assign
    {
        template<typename T, typename U = T>
            requires std::assignable_from<T&, U>
        constexpr decltype(auto) operator()(T& left, U&& right) const //
            noexcept(nothrow_assignable_from<T&, U>)
        {
            return left = cpp_forward(right);
        }
    };

    struct assign_by_construct : assign
    {
        template<
            typename T,
            typename... Args,
            std::constructible_from<Args...> DecayT = std::remove_reference_t<T>>
            requires std::invocable<assign, T&, DecayT>
        constexpr decltype(auto) operator()(T& left, Args&&... args) const noexcept(
            nothrow_constructible_from<DecayT, Args...> && //
            nothrow_invocable<assign, T&, DecayT>
        )
        {
            return (*this)(left, DecayT{cpp_forward(args)...});
        }
    };
}

namespace stdsharp
{
    inline constexpr struct assign :
        sequenced_invocables<details::assign, details::assign_by_construct>
    {
    } assign_v{};

    inline constexpr std::ranges::equal_to equal_to_v{};
    inline constexpr std::ranges::not_equal_to not_equal_to_v{};
    inline constexpr std::ranges::less less_v{};
    inline constexpr std::ranges::greater greater_v{};
    inline constexpr std::ranges::less_equal less_equal_v{};
    inline constexpr std::ranges::greater_equal greater_equal_v{};
    inline constexpr std::compare_three_way compare_three_way_v{};
    inline constexpr std::plus<> plus_v{};
    inline constexpr std::minus<> minus_v{};
    inline constexpr std::divides<> divides_v{};
    inline constexpr std::multiplies<> multiplies_v{};
    inline constexpr std::modulus<> modulus_v{};
    inline constexpr std::negate<> negate_v{};
    inline constexpr std::logical_and<> logical_and_v{};
    inline constexpr std::logical_not<> logical_not_v{};
    inline constexpr std::logical_or<> logical_or_v{};

    inline constexpr std::bit_and<> bit_and_v{};
    inline constexpr std::bit_not<> bit_not_v{};
    inline constexpr std::bit_or<> bit_or_v{};
    inline constexpr std::bit_xor<> bit_xor_v{};
    using bit_xnor = composed<std::bit_xor<>, std::bit_not<>>;
    inline constexpr bit_xnor bit_xnor_v{};

#define SHARP_SHIFT_OPERATE(direction, operate)                                           \
    inline constexpr struct direction##_shift                                             \
    {                                                                                     \
        template<typename T, typename U = T>                                              \
        [[nodiscard]] constexpr decltype(auto) operator()(T&& left, U&& right) const /**/ \
            noexcept(noexcept(cpp_forward(left) operate cpp_forward(right)))              \
            requires requires { cpp_forward(left) operate cpp_forward(right); }           \
        {                                                                                 \
            return cpp_forward(left) operate cpp_forward(right);                          \
        }                                                                                 \
    } direction##_shift_v{};

    SHARP_SHIFT_OPERATE(left, <<)
    SHARP_SHIFT_OPERATE(right, >>)

#undef SHARP_SHIFT_OPERATE

#define STDSHARP_ASSIGN_OPERATE(operator_type, op)                                               \
    template<typename T, typename U>                                                             \
    concept operator_type##_assignable_from = requires(T t, U&& u) { t op## = cpp_forward(u); }; \
                                                                                                 \
    namespace details                                                                            \
    {                                                                                            \
        struct operator_type##_assign                                                            \
        {                                                                                        \
            template<typename T, typename U = T>                                                 \
                requires(operator_type##_assignable_from<T, U>)                                  \
            constexpr decltype(auto) operator()(T& t, U&& u) const /**/                          \
                noexcept(noexcept((t op## = cpp_forward(u))))                                    \
            {                                                                                    \
                return t op## = cpp_forward(u);                                                  \
            }                                                                                    \
        };                                                                                       \
                                                                                                 \
        struct indirect_##operator_type##_assign                                                 \
        {                                                                                        \
            template<typename T, typename U = T>                                                 \
            constexpr decltype(auto) operator()(T& t, U&& u) const /**/                          \
                noexcept(noexcept((t = operator_type##_v(t, cpp_forward(u)))))                   \
                requires requires { t = operator_type##_v(t, cpp_forward(u)); }                  \
            {                                                                                    \
                return t = operator_type##_v(t, cpp_forward(u));                                 \
            }                                                                                    \
        };                                                                                       \
    }                                                                                            \
                                                                                                 \
    using operator_type##_assign = sequenced_invocables<                                         \
        details::operator_type##_assign,                                                         \
        details::indirect_##operator_type##_assign>;                                             \
                                                                                                 \
    inline constexpr operator_type##_assign operator_type##_assign_v{};

    STDSHARP_ASSIGN_OPERATE(plus, +)
    STDSHARP_ASSIGN_OPERATE(minus, -)
    STDSHARP_ASSIGN_OPERATE(divides, /)
    STDSHARP_ASSIGN_OPERATE(multiplies, *)
    STDSHARP_ASSIGN_OPERATE(modulus, %)
    STDSHARP_ASSIGN_OPERATE(bit_and, &)
    STDSHARP_ASSIGN_OPERATE(bit_or, |)
    STDSHARP_ASSIGN_OPERATE(bit_xor, ^)
    STDSHARP_ASSIGN_OPERATE(left_shift, <<)
    STDSHARP_ASSIGN_OPERATE(right_shift, >>)

#undef STDSHARP_ASSIGN_OPERATE

#define STDSHARP_ASSIGN_OPERATE(operator_type)                                          \
    inline constexpr struct operator_type##_assign                                      \
    {                                                                                   \
        template<typename T, typename U = T>                                            \
            requires requires(T t, U&& u) { t = operator_type##_v(t, cpp_forward(u)); } \
        constexpr decltype(auto) operator()(T& t, U&& u) const /**/                     \
            noexcept(noexcept((t = operator_type##_v(t, cpp_forward(u)))))              \
        {                                                                               \
            return t = operator_type##_v(t, cpp_forward(u));                            \
        }                                                                               \
    } operator_type##_assign_v{};

    STDSHARP_ASSIGN_OPERATE(negate)
    STDSHARP_ASSIGN_OPERATE(logical_and)
    STDSHARP_ASSIGN_OPERATE(logical_not)
    STDSHARP_ASSIGN_OPERATE(logical_or)

#undef STDSHARP_ASSIGN_OPERATE

    template<typename T>
    struct identity_with_fn
    {
        [[nodiscard]] constexpr decltype(auto) operator()(T&& t) const noexcept
        {
            return cpp_forward(t);
        }
    };

    template<typename T>
    inline constexpr identity_with_fn<T> identity_with_v{};

    inline constexpr std::identity identity_v{};

#define STDSHARP_INC_DEC_OPERATE(operator_prefix, op, al_op)                                       \
    inline constexpr struct pre_##operator_prefix##crease                                          \
    {                                                                                              \
        template<typename T>                                                                       \
        constexpr decltype(auto) operator()(T& t) const noexcept(noexcept(op##op t))               \
            requires requires { op##op t; }                                                        \
        {                                                                                          \
            return op##op t;                                                                       \
        }                                                                                          \
    } pre_##operator_prefix##crease_v{};                                                           \
                                                                                                   \
    inline constexpr struct post_##operator_prefix##crease                                         \
    {                                                                                              \
        template<weakly_decrementable T>                                                           \
        [[nodiscard]] constexpr decltype(auto) operator()(T& t) const noexcept(noexcept(t op##op)) \
            requires requires { t op##op; }                                                        \
        {                                                                                          \
            return t op##op;                                                                       \
        }                                                                                          \
    } post_##operator_prefix##crease_v{};

    STDSHARP_INC_DEC_OPERATE(in, +, plus)
    STDSHARP_INC_DEC_OPERATE(de, -, minus)

#undef STDSHARP_INC_DEC_OPERATE
}

namespace stdsharp::details
{
    struct advance_by_op
    {
        template<typename T, std::signed_integral Distance = std::iter_difference_t<T>>
            requires(std::invocable<pre_increase, T&> && std::invocable<pre_decrease, T&>)
        constexpr decltype(auto) operator()(T& v, Distance distance) const noexcept(
            noexcept(nothrow_invocable<pre_increase, T&> && nothrow_invocable<pre_decrease, T&>)
        )
        {
            for(; distance > 0; --distance) pre_increase_v(v);
            for(; distance < 0; ++distance) pre_decrease_v(v);

            return v;
        }
    };
}

namespace stdsharp
{
    using advance = sequenced_invocables<plus_assign, details::advance_by_op>;
    inline constexpr advance advance_v{};

    inline constexpr struct logical_imply_fn
    {
        [[nodiscard]] constexpr auto operator()(const bool first_cond, const bool second_cond) //
            const noexcept
        {
            return !first_cond || second_cond;
        }
    } logical_imply{};

#define STDSHARP_OPERATOR(name)          \
    using name##_fn = std::ranges::name; \
    inline constexpr name##_fn name{};

    STDSHARP_OPERATOR(equal_to)
    STDSHARP_OPERATOR(not_equal_to)
    STDSHARP_OPERATOR(less)
    STDSHARP_OPERATOR(greater)
    STDSHARP_OPERATOR(less_equal)
    STDSHARP_OPERATOR(greater_equal)
#undef STDSHARP_OPERATOR

#define STDSHARP_OPERATOR(name)  \
    using name##_fn = std::name; \
    inline constexpr name##_fn name{};

    STDSHARP_OPERATOR(compare_three_way)
    STDSHARP_OPERATOR(identity)

#undef STDSHARP_OPERATOR

#define STDSHARP_OPERATOR(name)    \
    using name##_fn = std::name<>; \
    inline constexpr name##_fn name{};

    STDSHARP_OPERATOR(plus)
    STDSHARP_OPERATOR(minus)
    STDSHARP_OPERATOR(divides)
    STDSHARP_OPERATOR(multiplies)
    STDSHARP_OPERATOR(modulus)
    STDSHARP_OPERATOR(negate)
    STDSHARP_OPERATOR(logical_and)
    STDSHARP_OPERATOR(logical_not)
    STDSHARP_OPERATOR(logical_or)
    STDSHARP_OPERATOR(bit_and)
    STDSHARP_OPERATOR(bit_not)
    STDSHARP_OPERATOR(bit_or)
    STDSHARP_OPERATOR(bit_xor)
#undef STDSHARP_OPERATOR

    inline constexpr struct bit_xnor_fn
    {
        template<typename T, typename U>
            requires(
                std::invocable<bit_xor_fn, T, U> &&
                std::invocable<bit_not_fn, std::invoke_result_t<bit_xor_fn, T, U>>
            )
        static constexpr decltype(auto) operator()(T&& t, U&& u) noexcept(
            nothrow_invocable<bit_xor_fn, T, U> &&
            nothrow_invocable<bit_not_fn, std::invoke_result_t<bit_xor_fn, T, U>>
        )
        {
            return bit_not(bit_xor(cpp_forward(t), cpp_forward(u)));
        }
    } bit_xnor{};

#define STDSHARP_OPERATOR(direction, operate)                                              \
    inline constexpr struct direction##_shift_fn                                           \
    {                                                                                      \
        template<typename T, typename U = T>                                               \
        [[nodiscard]] static constexpr decltype(auto) operator()(T&& left, U&& right) /**/ \
            noexcept(noexcept(cpp_forward(left) operate cpp_forward(right)))               \
            requires requires { cpp_forward(left) operate cpp_forward(right); }            \
        {                                                                                  \
            return cpp_forward(left) operate cpp_forward(right);                           \
        }                                                                                  \
    } direction##_shift{};

    STDSHARP_OPERATOR(left, <<)
    STDSHARP_OPERATOR(right, >>)

#undef STDSHARP_OPERATOR

#define STDSHARP_OPERATOR(operator_type, op)                                                     \
    template<typename T, typename U>                                                             \
    concept operator_type##_assignable_from = requires(T t, U&& u) { t op## = cpp_forward(u); }; \
                                                                                                 \
    struct operator_type##_assign_fn                                                             \
    {                                                                                            \
        template<typename T, typename U = T>                                                     \
            requires(operator_type##_assignable_from<T, U>)                                      \
        constexpr decltype(auto) operator()(T& t, U&& u) const /**/                              \
            noexcept(noexcept((t op## = cpp_forward(u))))                                        \
        {                                                                                        \
            return t op## = cpp_forward(u);                                                      \
        }                                                                                        \
                                                                                                 \
        template<typename T, typename U = T>                                                     \
        constexpr decltype(auto) operator()(T& t, U&& u) const /**/                              \
            noexcept(noexcept((t = operator_type(t, cpp_forward(u)))))                           \
            requires requires {                                                                  \
                t = operator_type(t, cpp_forward(u));                                            \
                requires !operator_type##_assignable_from<T, U>;                                 \
            }                                                                                    \
        {                                                                                        \
            return t = operator_type(t, cpp_forward(u));                                         \
        }                                                                                        \
    };                                                                                           \
                                                                                                 \
    inline constexpr operator_type##_assign_fn operator_type##_assign{};

    STDSHARP_OPERATOR(plus, +)
    STDSHARP_OPERATOR(minus, -)
    STDSHARP_OPERATOR(divides, /)
    STDSHARP_OPERATOR(multiplies, *)
    STDSHARP_OPERATOR(modulus, %)
    STDSHARP_OPERATOR(bit_and, &)
    STDSHARP_OPERATOR(bit_or, |)
    STDSHARP_OPERATOR(bit_xor, ^)
    STDSHARP_OPERATOR(left_shift, <<)
    STDSHARP_OPERATOR(right_shift, >>)

#undef STDSHARP_OPERATOR

#define STDSHARP_OPERATOR(operator_type)                                            \
    inline constexpr struct operator_type##_assign_fn                               \
    {                                                                               \
        template<typename T, typename U = T>                                        \
            requires requires(T t, U&& u) { t = operator_type(t, cpp_forward(u)); } \
        constexpr decltype(auto) operator()(T& t, U&& u) const /**/                 \
            noexcept(noexcept((t = operator_type(t, cpp_forward(u)))))              \
        {                                                                           \
            return t = operator_type(t, cpp_forward(u));                            \
        }                                                                           \
    } operator_type##_assign{};

    STDSHARP_OPERATOR(logical_and)
    STDSHARP_OPERATOR(logical_not)
    STDSHARP_OPERATOR(logical_or)
    STDSHARP_OPERATOR(logical_imply)
    STDSHARP_OPERATOR(compare_three_way)

#undef STDSHARP_OPERATOR

#define STDSHARP_OPERATOR(operator_prefix, op, al_op)                                              \
    inline constexpr struct pre_##operator_prefix##crease_fn                                       \
    {                                                                                              \
        template<typename T>                                                                       \
        constexpr decltype(auto) operator()(T& t) const noexcept(noexcept(op##op t))               \
            requires requires { op##op t; }                                                        \
        {                                                                                          \
            return op##op t;                                                                       \
        }                                                                                          \
    } pre_##operator_prefix##crease{};                                                             \
                                                                                                   \
    inline constexpr struct post_##operator_prefix##crease_fn                                      \
    {                                                                                              \
        template<typename T>                                                                       \
        [[nodiscard]] constexpr decltype(auto) operator()(T& t) const noexcept(noexcept(t op##op)) \
            requires requires { t op##op; }                                                        \
        {                                                                                          \
            return t op##op;                                                                       \
        }                                                                                          \
    } post_##operator_prefix##crease{};

    STDSHARP_OPERATOR(in, +, plus)
    STDSHARP_OPERATOR(de, -, minus)

#undef STDSHARP_OPERATOR

    inline constexpr struct advance_fn
    {
        template<typename T, std::signed_integral Distance = std::iter_difference_t<T>>
            requires std::invocable<plus_assign_fn, T&, const Distance&>
        constexpr decltype(auto) operator()(T& v, const Distance& distance) const //
            noexcept(nothrow_invocable<plus_assign_fn, T&, const Distance&>)
        {
            return plus_assign(v, distance);
        }

        template<typename T, std::signed_integral Distance = std::iter_difference_t<T>>
            requires(
                std::invocable<pre_increase_fn, T&> &&
                std::invocable<pre_decrease_fn, T&> &&
                !std::invocable<plus_assign_fn, T&, const Distance&>
            )
        constexpr decltype(auto) operator()(T& v, Distance distance) const noexcept(noexcept(
            nothrow_invocable<pre_increase_fn, T&> && nothrow_invocable<pre_decrease_fn, T&>
        ))
        {
            for(; distance > 0; --distance) pre_increase(v);
            for(; distance < 0; ++distance) pre_decrease(v);

            return v;
        }
    } advance{};

    inline constexpr struct indexer_fn
    {
        [[nodiscard]] static constexpr decltype(auto) operator()(
            auto&& element,
            auto&&... args
        ) noexcept(noexcept(cpp_forward(element)[cpp_forward(args)...]))
            requires requires { cpp_forward(element)[cpp_forward(args)...]; }
        {
            return cpp_forward(element)[cpp_forward(args)...];
        }
    } indexer{};
}
