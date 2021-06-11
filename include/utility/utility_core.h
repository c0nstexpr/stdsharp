// Created by BlurringShadow at 2021-02-27-下午 10:24

#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <random>
#include <type_traits>

#include <gsl/gsl>

namespace blurringshadow::utility
{
    using namespace std::literals;

    inline constexpr auto equal_to_v = std::ranges::equal_to{};
    inline constexpr auto not_equal_to_v = std::ranges::not_equal_to{};
    inline constexpr auto less_v = std::ranges::less{};
    inline constexpr auto greater_v = std::ranges::greater{};
    inline constexpr auto less_equal_v = std::ranges::less_equal{};
    inline constexpr auto greater_equal_v = std::ranges::greater_equal{};

    struct increment
    {
        constexpr auto operator()(auto&& v) const noexcept(noexcept(++v)) { return ++v; }
    };

    struct decrement
    {
        constexpr auto operator()(auto&& v) const noexcept(noexcept(--v)) { return --v; }
    };

    inline constexpr auto increment_v = increment{};
    inline constexpr auto decrement_v = decrement{};

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

    inline constexpr auto identity_v = std::identity{};

    template<std::default_initializable Functor, typename... Args>
        requires std::invocable<Functor, Args...>
    constexpr auto functor_invoke(Args&&... args)
    {
        // clang-format off
        if constexpr(auto functor = Functor{};
                     std::same_as<std::invoke_result<Functor, Args...>, void>)
            functor(std::forward<Args>(args)...);
       else return functor(std::forward<Args>(args)...);
        // clang-format on
    }

    template<typename T>
    struct auto_cast
    {
        T&& t;

        // clang-format off
        template<typename U>
            requires requires { static_cast<std::decay_t<U>>(std::forward<T>(t)); }
        [[nodiscard]] constexpr operator U()
            noexcept(std::is_nothrow_constructible_v<T, std::decay_t<U>>)
        {
            return static_cast<std::decay_t<U>>(std::forward<T>(t));
        }
        // clang-format on
    };

    template<typename T>
    auto_cast(T&& t) -> auto_cast<T>;

    template<typename T, typename U, typename Comp>
    constexpr T& set_if(T& left, U&& right, Comp&& comp)
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

    // clang-format off
    template<typename T, typename Compare>
    [[nodiscard]] constexpr bool is_between(
        const T& v,
        const std::type_identity_t<T>& min,
        const std::type_identity_t<T>& max,
       Compare&& cmp
    )
    // clang-format on
    {
        return std::addressof(std::clamp(v, min, max, std::forward<Compare>(cmp))) ==
            std::addressof(v);
    }

    // clang-format off
    template<typename T>
    [[nodiscard]] constexpr bool is_between(
        const T& v, const std::type_identity_t<T>& min, 
        const std::type_identity_t<T>& max
    )
    // clang-format on
    {
        return is_between(v, min, max, std::less<>{});
    }

    [[nodiscard]] inline auto& get_random_device()
    {
        static thread_local std::random_device random_device;
        return random_device;
    }

    template<typename T>
        requires std::is_enum_v<T>
    [[nodiscard]] constexpr auto to_underlying(const T v)
    {
        return static_cast<std::underlying_type_t<T>>(v);
    }
}
