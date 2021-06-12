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

    namespace details
    {
        template<typename T>
        concept random_incrementable = requires(T t, std::size_t i)
        {
            plus_v(t + i);
        };

        template<typename T>
        concept random_decrementable = requires(T t, std::size_t i)
        {
            minus_v(t - i);
        };

        template<typename T>
        concept has_increase_cpo = requires(T t)
        {
            increase(t);
        };

        template<typename T>
        concept has_decrease_cpo = requires(T t)
        {
            decrease(t);
        };

        template<typename T>
        concept has_random_increase_cpo = requires(T t, std::size_t i)
        {
            increase(t, i);
        };

        template<typename T>
        concept has_random_decrease_cpo = requires(T t, std::size_t i)
        {
            decrease(t, i);
        };

        inline constexpr auto increase_impl = []<typename T>(T&& v, std::size_t i)
        {
            using real_t = std::remove_cvref_t<T>;

            if constexpr(has_random_increase_cpo<real_t>) return increase(std::forward<T>(v), i);
            else if constexpr(random_incrementable<real_t>)
                return plus_v(std::forward<T>(v), i);
            else
            {
                auto res = v;

                // clang-format off
                for(; i > 0; --i)
                    if constexpr(has_increase_cpo<real_t>) res = increase(res);
                    else ++res;
                // clang-format on
                return res;
            }
        };

        inline constexpr auto decrease_impl = []<typename T>(T&& v, std::size_t i)
        {
            using real_t = std::remove_cvref_t<T>;

            if constexpr(has_random_decrease_cpo<real_t>) return decrease(std::forward<T>(v), i);
            else if constexpr(random_decrementable<real_t>)
                return minus_v(std::forward<T>(v), i);
            else
            {
                auto res = v;

                // clang-format off
                for(; i > 0; --i)
                    if constexpr(has_decrease_cpo<real_t>) res = decrease(res);
                    else --res;
                // clang-format on
                return res;
            }
        };

    }

    struct increase
    {
        template<typename T>
        [[nodiscard]] constexpr auto operator()(T&& v, const std::size_t i = 1) const
            noexcept(noexcept(details::increase_impl(std::forward<T>(v), i)))
        {
            return details::increase_impl(std::forward<T>(v), i);
        }
    };

    struct decrease
    {
        template<typename T>
        [[nodiscard]] constexpr auto operator()(T&& v, const std::size_t i = 1) const
            noexcept(noexcept(details::decrease_impl(std::forward<T>(v), i)))
        {
            return details::decrease_impl(std::forward<T>(v), i);
        }
    };

    inline constexpr auto increase_v = increase{};
    inline constexpr auto decrease_v = decrease{};

    namespace details
    {
        template<typename T, typename Distance>
        concept has_advance_cpo = requires(T& t, Distance d)
        {
            advance(t, d);
        };

        inline constexpr auto advance_impl = []<typename T, typename Distance>(T& v, Distance&& i)
        {
            // clang-format off
            if constexpr(details::has_advance_cpo<std::remove_cvref_t<T>, Distance>)
                return advance(v, std::forward<Distance>(i));
            else return v += i;
            // clang-format on
        };

    }

    struct advance
    {
        template<typename T, typename Distance>
        [[nodiscard]] constexpr auto operator()(T& v, Distance&& distance) const
            noexcept(noexcept(details::advance_impl(v, distance)))
        {
            return details::advance_impl(v, std::forward<Distance>(distance));
        }
    };

    inline constexpr auto advance_v = advance{};

    template<std::default_initializable Functor, typename... Args>
        requires std::invocable<Functor, Args...>
    constexpr auto functor_invoke(Args&&... args) { return Functor{}(std::forward<Args>(args)...); }

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
