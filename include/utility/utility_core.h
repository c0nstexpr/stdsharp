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

    // clang-format off
    inline constexpr auto left_shift = []<typename T, typename U>(T&& left, U&& right)
        noexcept(noexcept(std::forward<T>(left) << std::forward<U>(right)))
    {
        return std::forward<T>(left) << std::forward<U>(right);
    };

    inline constexpr auto right_shift = []<typename T, typename U>(T&& left, U&& right)
        noexcept(noexcept(std::forward<T>(left) >> std::forward<U>(right)))
    {
        return std::forward<T>(left) >> std::forward<U>(right);
    };
    // clang-format on

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

#undef BS_UTIL_ASSIGN_OPERATE

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

        // clang-format off
        static inline constexpr auto increase_impl = []<typename T>(T&& v, std::size_t i) noexcept(
            has_random_increase_cpo<T> && noexcept(increase(std::forward<T>(v), i)) ||
            random_incrementable<T> &&
                std::is_nothrow_invocable_v<std::plus<>, T, std::size_t> ||
            (!has_increase_cpo<T> ||
                noexcept(increase(v)) &&
                std::is_nothrow_assignable_v<T, std::add_rvalue_reference_t<decltype(increase(v))>>))
        // clang-format on
        {
            if constexpr(has_random_increase_cpo<T>) return increase(std::forward<T>(v), i);
            else if constexpr(random_incrementable<T>)
                return plus_v(std::forward<T>(v), i);
            else
            {
                auto res = std::forward<T>(v);

                // clang-format off
                for(; i > 0; --i)
                    if constexpr(has_increase_cpo<T>) res = increase(res);
                    else ++res;
                // clang-format on
                return res;
            }
        };

        // clang-format off
        static inline constexpr auto decrease_impl = []<typename T>(T&& v, std::size_t i) noexcept(
            has_random_increase_cpo<T> && noexcept(increase(std::forward<T>(v), i)) ||
            random_decrementable<T> &&
                std::is_nothrow_invocable_v<std::minus<>, T, std::size_t> ||
            (!has_decrease_cpo<T> ||
                noexcept(decrease(v)) &&
                std::is_nothrow_assignable_v<T, std::add_rvalue_reference_t<decltype(decrease(v))>>))
        // clang-format on
        {
            if constexpr(has_random_decrease_cpo<T>) return decrease(std::forward<T>(v), i);
            else if constexpr(random_decrementable<T>)
                return minus_v(std::forward<T>(v), i);
            else
            {
                auto res = std::forward<T>(v);

                // clang-format off
                for(; i > 0; --i)
                    if constexpr(has_decrease_cpo<T>) res = decrease(res);
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

        // clang-format off
        inline constexpr auto advance_impl = []<typename T, typename Distance>(T& v, Distance&& i)
            noexcept(
                details::has_advance_cpo<T, Distance> && noexcept(advance(v, std::forward<Distance>(i))) ||
                noexcept(plus_assign(v, i)) ||
                noexcept(increase_v(v, i), decrease_v(v, i))
            )
        // clang-format on
        {
            if constexpr(details::has_advance_cpo<T, Distance>)
                return advance(v, std::forward<Distance>(i));
            else if constexpr(std::invocable<decltype(plus_assign), T&, Distance>)
                return plus_assign(v, i);
            else
            {
                if(i > 0) return increase_v(v, i);
                return decrease_v(v, i);
            }
        };

    }

    struct advance
    {
        template<typename Distance>
        [[nodiscard]] constexpr auto operator()(auto& v, Distance&& distance) const
            noexcept(noexcept(details::advance_impl(v, distance)))
        {
            return details::advance_impl(v, std::forward<Distance>(distance));
        }
    };

    inline constexpr auto advance_v = advance{};

    template<typename T>
    struct auto_cast
    {
        T&& t;


        template<typename U>
        // clang-format off
        [[nodiscard]] constexpr operator U() && 
            noexcept(noexcept(static_cast<U>(std::forward<T>(t))))
        // clang-format on
        {
            return static_cast<U>(std::forward<T>(t));
        }
    };

    template<typename T>
    auto_cast(T&& t) -> auto_cast<T>;

    // clang-format off
    template<typename T, typename U, std::predicate<T, U> Comp>
    constexpr T& set_if(T& left, U&& right, Comp&& comp)
        noexcept(
            std::is_nothrow_invocable_r_v<bool, Comp, U, T&> &&
            std::is_nothrow_assignable_v<T, U>
        )
    // clang-format on
    {
        if(std::invoke(std::forward<Comp>(comp), std::forward<U>(right), left))
            left = std::forward<U>(right);
        return left;
    }

    // clang-format off
    template<typename T, typename U>
    constexpr T& set_if_greater(T& left, U&& right)
        noexcept(noexcept(set_if(left, std::forward<U>(right), greater_v)))
    // clang-format on
    {
        return set_if(left, std::forward<U>(right), greater_v);
    }

    // clang-format off
    template<typename T, typename U>
    constexpr T& set_if_less(T& left, U&& right)
        noexcept(noexcept(set_if(left, std::forward<U>(right), less_v)))
    // clang-format on
    {
        return set_if(left, std::forward<U>(right), less_v);
    }

    namespace details
    {
        template<typename T, typename U, typename V, typename Compare>
        struct is_between_requirement : std::type_identity<std::common_type_t<T, U, V>>
        {
            // clang-format off
            static constexpr auto value = std::predicate<
                Compare,
                typename is_between_requirement::type,
                typename is_between_requirement::type
            >;
            // clang-format on
        };

        template<typename T, typename Compare>
        // clang-format off
        [[nodiscard]] constexpr bool is_between_impl(
            const T& v,
            const T& min,
            const T& max,
            Compare&& cmp
        ) noexcept(noexcept(std::clamp(v, min, max, std::forward<Compare>(cmp))))
        // clang-format on
        {
            return std::addressof(std::clamp(v, min, max, std::forward<Compare>(cmp))) ==
                std::addressof(v);
        }
    }

    // clang-format off
    template<typename T, typename U, typename V, typename Compare>
        requires details::is_between_requirement<T, U, V, Compare>::value
    [[nodiscard]] constexpr bool is_between(const T& v, const U& min, const V& max, Compare&& cmp)
        noexcept(
            noexcept(
                details::is_between_impl(
                    static_cast<std::common_type_t<T, U, V>>(v),
                    static_cast<std::common_type_t<T, U, V>>(min),
                    static_cast<std::common_type_t<T, U, V>>(max),
                    std::forward<Compare>(cmp)
                )
            )
        )
    {
        using common_t = std::common_type_t<T, U, V>;

        return details::is_between_impl(
            static_cast<common_t>(v),
            static_cast<common_t>(min),
            static_cast<common_t>(max),
            std::forward<Compare>(cmp)
        );
    }
    // clang-format on

    // clang-format off
    template<typename T, typename U, typename V>
    [[nodiscard]] constexpr bool is_between(const T& v, const U& min, const V& max)
        noexcept(noexcept(is_between(v, min, max, less_v)))
    // clang-format on
    {
        return is_between(v, min, max, less_v);
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
}
