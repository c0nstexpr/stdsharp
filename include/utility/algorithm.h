#pragma once

#include <algorithm>

#include "functional.h"

namespace blurringshadow::utility
{
    // clang-format off
    inline constexpr auto set_if = []<typename T, typename U, std::predicate<T, U> Comp>(
        T& left,
        U&& right,
        Comp comp
    ) noexcept(
        std::is_nothrow_invocable_r_v<bool, Comp, U&, T&> &&
        std::is_nothrow_assignable_v<T, U>
    )  -> T& // clang-format on
    {
        if(std::invoke(std::move(comp), right, left)) left = std::forward<U>(right);
        return left;
    };

    // clang-format off
    inline constexpr auto set_if_greater = []<typename T, typename U>(T& left, U&& right)
        noexcept(noexcept(set_if(left, std::forward<U>(right), greater_v))) -> T& 
    {
        return set_if(left, std::forward<U>(right), greater_v);
    }; // clang-format on

    // clang-format off
    inline constexpr auto  set_if_less = []<typename T, typename U>(T& left, U&& right)
        noexcept(noexcept(set_if(left, std::forward<U>(right), less_v))) -> T& 
    {
        return set_if(left, std::forward<U>(right), less_v);
    }; // clang-format on

    namespace details
    {
        struct is_between_fn
        {
            // clang-format off
            template<
                typename T,
                typename U,
                typename V,
                std::predicate<
                    std::common_type_t<T, U, V>, std::common_type_t<T, U, V>
                > Compare
            > // clang-format on
            [[nodiscard]] constexpr bool operator()(
                const T& v,
                const U& min,
                const V& max,
                Compare&& cmp // clang-format off
            ) const // clang-format on
            {
                using common_t = std::common_type_t<T, U, V>;

                const auto cast_v = static_cast<common_t>(v);
                const auto cast_min = static_cast<common_t>(min);
                const auto cast_max = static_cast<common_t>(max);

                return std::addressof(std::ranges::clamp(
                           cast_v,
                           cast_min,
                           cast_max,
                           std::forward<Compare>(cmp) // clang-format off
                )) == std::addressof(cast_v); // clang-format on
            }
            template<typename T, typename U, typename V>
            [[nodiscard]] constexpr bool operator()(const T& v, const U& min, const V& max) const
            {
                return operator()(v, min, max, less_v);
            }
        };
    }

    inline constexpr details::is_between_fn is_between{};
}
