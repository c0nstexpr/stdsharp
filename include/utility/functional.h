#pragma once

#include <functional>

#include "type_traits.h"

namespace blurringshadow::utility
{
    inline constexpr auto equal_to_v = std::ranges::equal_to{};
    inline constexpr auto not_equal_to_v = std::ranges::not_equal_to{};
    inline constexpr auto less_v = std::ranges::less{};
    inline constexpr auto greater_v = std::ranges::greater{};
    inline constexpr auto less_equal_v = std::ranges::less_equal{};
    inline constexpr auto greater_equal_v = std::ranges::greater_equal{};
    inline constexpr auto compare_three_way_v = std::compare_three_way{};

    inline constexpr auto plus_v = std::plus<>{};
    inline constexpr auto minus_v = std::minus<>{};
    inline constexpr auto divides_v = std::divides<>{};
    inline constexpr auto multiplies_v = std::multiplies<>{};
    inline constexpr auto modulus_v = std::modulus<>{};
    inline constexpr auto negate_v = std::negate<>{};

    inline constexpr auto logical_and_v = std::logical_and<>{};
    inline constexpr auto logical_not_v = std::logical_not<>{};
    inline constexpr auto logical_or_v = std::logical_or<>{};

    inline constexpr auto bit_and_v = std::bit_and<>{};
    inline constexpr auto bit_not_v = std::bit_not<>{};
    inline constexpr auto bit_or_v = std::bit_or<>{};
    inline constexpr auto bit_xor_v = std::bit_xor<>{};

    struct left_shift
    {
        template<typename T, typename U>
        // clang-format off
        constexpr auto operator()(T&& left, U&& right) noexcept(
            noexcept(std::forward<T>(left) << std::forward<U>(right))
        ) // clang-format on
        {
            return std::forward<T>(left) << std::forward<U>(right);
        }
    };

    struct right_shift
    {
        template<typename T, typename U>
        // clang-format off
        constexpr auto operator()(T&& left, U&& right) noexcept(
            noexcept(std::forward<T>(left) >> std::forward<U>(right))
        ) // clang-format on
        {
            return std::forward<T>(left) >> std::forward<U>(right);
        }
    };

    inline constexpr auto left_shift_v = left_shift{};

    inline constexpr auto right_shift_v = right_shift{};

    namespace details
    {
        template<bool Requirement, auto Operation, auto AlternativeOperation>
        // clang-format off
       inline constexpr auto operator_assign = []<typename T>(auto& left, T&& right) noexcept(
            Requirement&& noexcept(Operation(left, std::forward<T>(right))) ||
            noexcept(left = AlternativeOperation(left, std::forward<T>(right)))
       )
        // clang-format on
        {
            if constexpr(Requirement) return Operation(left, std::forward<T>(right));
            else
            {
                left = AlternativeOperation(left, std::forward<T>(right));
                return left;
            }
        };
    }

#define BS_UTIL_ASSIGN_OPERATE(operator_type, op)                                            \
    namespace details                                                                        \
    {                                                                                        \
        template<typename T, typename U>                                                     \
        concept operator_type##_assignable = requires(T a, U b)                              \
        {                                                                                    \
            a op## = b;                                                                      \
        };                                                                                   \
    }                                                                                        \
    inline constexpr auto operator_type##_assign =                                           \
        []<typename T>(auto& left, T&& right) noexcept(                                      \
            noexcept(details::operator_assign<                                               \
                     details::operator_type##_assignable<decltype(left), T>,                 \
                     []<typename U>(auto& l, U&& r) { return l op## = std::forward<U>(r); }, \
                     operator_type##_v>(left, std::forward<T>(right))))                      \
    {                                                                                        \
        return details::operator_assign<                                                     \
            details::operator_type##_assignable<decltype(left), T>,                          \
            []<typename U>(auto& l, U&& r) { return l op## = std::forward<U>(r); },          \
            operator_type##_v>(left, std::forward<T>(right));                                \
    };

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

    inline constexpr auto identity_v = std::identity{};

    namespace details
    {
        template<typename T>
        concept has_increase_cpo = requires(T t, const std::size_t i)
        {
            increase(t, i);
        };

        template<typename T>
        // clang-format off
        [[nodiscard]] constexpr auto invoke_increase_cpo(T&& v, const std::size_t i)
            noexcept(noexcept(increase(std::forward<T>(v), i)))
        // clang-format on
        {
            return increase(std::forward<T>(v), i);
        }

        template<typename T>
        concept has_decrease_cpo = requires(T t, const std::size_t i)
        {
            decrease(t, i);
        };

        template<typename T>
        // clang-format off
        [[nodiscard]] constexpr auto invoke_decrease_cpo(T&& v, const std::size_t i)
            noexcept(noexcept(decrease(std::forward<T>(v), i)))
        // clang-format on
        {
            return decrease(std::forward<T>(v), i);
        }

    }

    struct increase
    {
        template<details::has_increase_cpo T>
        // clang-format off
        [[nodiscard]] constexpr auto operator()(T v, const std::size_t i) const noexcept(
            nothrow_invocable<&details::invoke_increase_cpo, T, std::size_t>
        )
        // clang-format on
        {
            return details::invoke_increase_cpo(std::move(v), i);
        }

        template<typename T>
            requires(!details::has_increase_cpo<T> && std::invocable<std::plus<>, T, std::size_t>)
        // clang-format off
        [[nodiscard]] constexpr auto operator()(T v, const std::size_t i) const noexcept(
            nothrow_invocable<std::plus<>, T, std::size_t>
        )
        // clang-format on
        {
            return plus_v(std::move(v), i);
        }

        template<typename T>
        [[nodiscard]] constexpr auto operator()(T v, std::size_t i) const noexcept(noexcept(++v))
        {
            for(; i > 0; --i) ++v;
            return v;
        }
    };

    struct decrease
    {
        template<details::has_decrease_cpo T>
        // clang-format off
        [[nodiscard]] constexpr auto operator()(T v, const std::size_t i) const noexcept(
            nothrow_invocable<&details::invoke_decrease_cpo, T, std::size_t>
        )
        // clang-format on
        {
            return details::invoke_decrease_cpo(std::move(v), i);
        }

        template<typename T>
            requires(!details::has_decrease_cpo<T> && std::invocable<std::minus<>, T, std::size_t>)
        // clang-format off
        [[nodiscard]] constexpr auto operator()(T v, const std::size_t i) const noexcept(
            nothrow_invocable<std::minus<>, T, std::size_t>
        )
        // clang-format on
        {
            return minus_v(std::move(v), i);
        }

        template<typename T>
        [[nodiscard]] constexpr auto operator()(T v, std::size_t i) const noexcept(noexcept(++v))
        {
            for(; i > 0; --i) --v;
            return v;
        }
    };

    inline constexpr auto increase_v = increase{};
    inline constexpr auto decrease_v = decrease{};

    namespace details
    {
        template<typename T, typename Distance>
        concept has_advance_cpo = requires(T& t, const Distance d)
        {
            advance(t, d);
        };

        template<typename T, typename Distance>
        constexpr auto invoke_advance_cpo(T& t, const Distance& d) noexcept(noexcept(advance(t, d)))
        {
            return advance(t, d);
        }
    }

    struct advance
    {
        template<typename T, typename Distance>
            requires details::has_advance_cpo<T, Distance>
        // clang-format off
        [[nodiscard]] constexpr auto operator()(T& v, const Distance& distance) const noexcept(
             noexcept(details::invoke_advance_cpo(v, distance))
        )
        // clang-format on
        {
            return details::invoke_advance_cpo(v, distance);
        }

        // clang-format off
        template<typename T, typename Distance>
            requires (
                !details::has_advance_cpo<T, Distance> &&
                std::invocable<std::plus<>, T, Distance>
            )
        [[nodiscard]] constexpr auto operator()(T& v, const Distance& distance) const noexcept(
            noexcept(plus_assign(v, distance))
        )
        // clang-format on
        {
            return plus_assign(v, distance);
        }

        template<typename T, typename Distance>
        // clang-format off
        [[nodiscard]] constexpr auto operator()(T& v, const Distance& distance) const noexcept(
            noexcept(increase_v(v, distance), decrease_v(v, distance))
        )
        // clang-format on
        {
            if(distance > 0) return increase_v(v, distance);
            return decrease_v(v, distance);
        }
    };

    inline constexpr auto advance_v = advance{};

    // clang-format off
    template<std::invocable... Func>
    [[nodiscard]] constexpr auto merge_invoke(Func&&... func)
        noexcept(noexcept(std::tuple{std::invoke(std::forward<Func>(func))...}))
    // clang-format on
    {
        return std::tuple{std::invoke(std::forward<Func>(func))...};
    }

    // clang-format off
    template<std::invocable... Func>
        requires(std::same_as<std::invoke_result_t<Func>, void> || ...)
    [[nodiscard]] constexpr auto merge_invoke(Func&&... func)
        noexcept((std::is_nothrow_invocable_v<Func> && ...))
    // clang-format on
    {
        (std::invoke(std::forward<Func>(func)), ...);
    }

    template<typename ReturnT, typename Func, typename... Args>
        requires std::invocable<Func, Args...> &&
            std::convertible_to<std::invoke_result_t<Func, Args...>, ReturnT>
    // clang-format off
    [[nodiscard]] constexpr ReturnT invoke_r(Func&& func, Args&&... args)
        noexcept(std::is_nothrow_invocable_r_v<ReturnT, Func, Args...>)
    // clang-format on
    {
        return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    }
}
