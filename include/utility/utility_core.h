// Created by BlurringShadow at 2021-02-27-下午 10:24

#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <random>
#include <type_traits>

#include <gsl/gsl>

using namespace std::literals;

namespace blurringshadow::utility
{
    inline constexpr auto equal_to = std::equal_to{};
    inline constexpr auto not_equal_to = std::not_equal_to{};
    inline constexpr auto less = std::less{};
    inline constexpr auto greater = std::greater{};
    inline constexpr auto less_equal = std::less_equal{};
    inline constexpr auto greater_equal = std::greater_equal{};

    inline constexpr auto plus = std::plus<>{};
    inline constexpr auto minus = std::minus<>{};
    inline constexpr auto divides = std::divides<>{};
    inline constexpr auto multiplies = std::multiplies<>{};
    inline constexpr auto modulus = std::modulus<>{};
    inline constexpr auto negate = std::negate<>{};

    inline constexpr auto logical_and = std::logical_and<>{};
    inline constexpr auto logical_not = std::logical_not<>{};
    inline constexpr auto logical_or = std::logical_or<>{};

    inline constexpr auto bit_and = std::bit_and<>{};
    inline constexpr auto bit_not = std::bit_not<>{};
    inline constexpr auto bit_or = std::bit_or<>{};
    inline constexpr auto bit_xor = std::bit_xor<>{};

    template<typename T>
    struct auto_cast
    {
        T&& t;

        template<typename U> requires requires { static_cast<std::decay_t<U>>(std::forward<T>(t)); }
        [[nodiscard]] constexpr operator
        U() noexcept(std::is_nothrow_constructible_v<T, std::decay_t<U>>)
        {
            return static_cast<std::decay_t<U>>(std::forward<T>(t));
        }
    };

    template<typename T>
    auto_cast(T&& t) -> auto_cast<T>;

    template<typename T, typename U, typename Comp>
    constexpr T& set_if(T& left, U&& right, Comp comp)
    {
        if(comp(right, left)) left = std::forward<U>(right);
        return left;
    }

    template<typename T, typename U>
    constexpr T& set_if_greater(T& left, U&& right)
    {
        return set_if(left, std::forward<U>(right), std::greater<>{});
    }

    template<typename T, typename U>
    constexpr T& set_if_lesser(T& left, U&& right)
    {
        return set_if(left, std::forward<U>(right), std::less<>{});
    }

    template<typename T, typename Compare>
    [[nodiscard]] constexpr bool is_between(
        const T& v,
        const std::type_identity_t<T>& min,
        const std::type_identity_t<T>& max,
        Compare cmp
    ) { return std::addressof(std::clamp(v, min, max, cmp)) == std::addressof(v); }

    template<typename T>
    [[nodiscard]] constexpr bool is_between(
        const T& v,
        const std::type_identity_t<T>& min,
        const std::type_identity_t<T>& max
    ) { return is_between(v, min, max, std::less<>{}); }

    [[nodiscard]] inline auto& get_random_device()
    {
        static std::random_device random_device;
        return random_device;
    }

    template<typename T> requires std::is_enum_v<T>
    [[nodiscard]] constexpr auto to_underlying(const T v) { return static_cast<std::underlying_type_t<T>>(v); }
}
