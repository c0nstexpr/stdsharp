#pragma once

#include <algorithm>

#include "functional.h"

namespace blurringshadow::utility
{
    // clang-format off
    template<typename T, typename U, std::predicate<T, U> Comp>
    constexpr T& set_if(T& left, U&& right, Comp comp)
        noexcept(
            std::is_nothrow_invocable_r_v<bool, Comp, U, T&> &&
            std::is_nothrow_assignable_v<T, U>
        )
    // clang-format on
    {
        if(std::invoke(std::move(comp), std::forward<U>(right), left))
            left = std::forward<U>(right);
        return left;
    }

    template<typename T, typename U>
        requires std::invocable<std::ranges::greater, U, T>
    // clang-format off
    constexpr T& set_if_greater(T& left, U&& right)
        noexcept(noexcept(set_if(left, std::forward<U>(right), greater_v)))
    // clang-format on
    {
        return set_if(left, std::forward<U>(right), greater_v);
    }

    template<typename T, typename U>
        requires std::invocable<std::ranges::less, U, T>
    // clang-format off
    constexpr T& set_if_less(T& left, U&& right)
        noexcept(noexcept(set_if(left, std::forward<U>(right), less_v)))
    // clang-format on
    {
        return set_if(left, std::forward<U>(right), less_v);
    }

    namespace details
    {
        template<typename T, typename U, typename V, typename Compare>
        struct is_between_fn
        {
            using common_t = std::common_type_t<T, U, V>;

            // clang-format off
            [[nodiscard]] constexpr bool operator()(
                const T& v,
                const U& min,
                const V& max,
                Compare&& cmp
            ) const requires std::predicate<Compare, common_t, common_t>
            // clang-format on
            {
                const auto cast_v = static_cast<common_t>(v);
                const auto cast_min = static_cast<common_t>(min);
                const auto cast_max = static_cast<common_t>(max);
                // clang-format off
                return std::addressof(
                    std::ranges::clamp(cast_v, cast_max, cast_min, std::forward<Compare>(cmp))
                ) == std::addressof(cast_v);
                // clang-format on
            }
        };
    }

    template<typename T, typename U, typename V, typename Compare>
        requires std::invocable<details::is_between_fn<T, U, V, Compare>, T, U, V, Compare>
    [[nodiscard]] constexpr bool is_between(const T& v, const U& min, const V& max, Compare&& cmp)
    {
        return details::is_between_fn<T, U, V, Compare>{}(v, min, max, std::forward<Compare>(cmp));
    }

    template<typename T, typename U, typename V>
        requires std::invocable<std::ranges::less, U, T> && std::invocable<std::ranges::less, T, V>
    [[nodiscard]] constexpr bool is_between(const T& v, const U& min, const V& max)
    {
        return is_between(v, min, max, less_v);
    }
}
