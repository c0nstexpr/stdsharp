#pragma once

#include <functional>

#include "type_traits.h"

namespace blurringshadow::utility
{
    inline constexpr std::ranges::equal_to equal_to_v;
    inline constexpr std::ranges::not_equal_to not_equal_to_v;
    inline constexpr std::ranges::less less_v;
    inline constexpr std::ranges::greater greater_v;
    inline constexpr std::ranges::less_equal less_equal_v;
    inline constexpr std::ranges::greater_equal greater_equal_v;
    inline constexpr std::compare_three_way compare_three_way_v;

    inline constexpr std::plus<> plus_v;
    inline constexpr std::minus<> minus_v;
    inline constexpr std::divides<> divides_v;
    inline constexpr std::multiplies<> multiplies_v;
    inline constexpr std::modulus<> modulus_v;
    inline constexpr std::negate<> negate_v;

    inline constexpr std::logical_and<> logical_and_v;
    inline constexpr std::logical_not<> logical_not_v;
    inline constexpr std::logical_or<> logical_or_v;

    inline constexpr std::bit_and<> bit_and_v;
    inline constexpr std::bit_not<> bit_not_v;
    inline constexpr std::bit_or<> bit_or_v;
    inline constexpr std::bit_xor<> bit_xor_v;

    struct left_shift
    {
        template<typename T, typename U> // clang-format off
        [[nodiscard]] constexpr auto operator()(T&& left, U&& right) const noexcept(
            noexcept(std::forward<T>(left) << std::forward<U>(right))
        ) // clang-format on
        {
            return std::forward<T>(left) << std::forward<U>(right);
        }
    };

    struct right_shift
    {
        template<typename T, typename U> // clang-format off
        [[nodiscard]] constexpr auto operator()(T&& left, U&& right) const noexcept(
            noexcept(std::forward<T>(left) >> std::forward<U>(right))
        ) // clang-format on
        {
            return std::forward<T>(left) >> std::forward<U>(right);
        }
    };

    inline constexpr left_shift left_shift_v{};

    inline constexpr right_shift right_shift_v{};

    namespace details
    {
        using namespace std;

        template<bool Requirement, auto Operation, auto AlternativeOperation> // clang-format off
        inline constexpr auto operator_assign = []<typename T>(auto& left, T&& right) noexcept(
            Requirement && noexcept(Operation(left, forward<T>(right))) ||
            noexcept(left = AlternativeOperation(left, forward<T>(right)))
        ) // clang-format on
        {
            if constexpr(Requirement) return Operation(left, forward<T>(right));
            else
            {
                left = AlternativeOperation(left, forward<T>(right));
                return left;
            }
        };
    }

#define BS_UTIL_ASSIGN_OPERATE(operator_type, op)                                     \
    namespace details                                                                 \
    {                                                                                 \
        using namespace std;                                                          \
                                                                                      \
        template<typename T, typename U>                                              \
        concept operator_type##_assignable = requires(T a, U b)                       \
        {                                                                             \
            a op## = b;                                                               \
        };                                                                            \
                                                                                      \
        struct operator_type##_assign_fn                                              \
        {                                                                             \
            template<typename T, typename U>                                          \
            static constexpr auto operator_assign_fn = operator_assign<               \
                operator_type##_assignable<T, U>,                                     \
                []<typename FwU>(T& l, FwU&& r) { return l op## = forward<FwU>(r); }, \
                operator_type##_v>;                                                   \
                                                                                      \
            template<typename T, typename U>                                          \
            [[nodiscard]] constexpr auto operator()(T& left, U&& right) const         \
                noexcept(noexcept(operator_assign_fn<T, U>(left, forward<U>(right)))) \
            {                                                                         \
                return operator_assign_fn<T, U>(left, forward<U>(right));             \
            }                                                                         \
        };                                                                            \
    }                                                                                 \
                                                                                      \
    inline constexpr details::operator_type##_assign_fn operator_type##_assign;

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

    inline constexpr std::identity identity_v;

    namespace details
    {
        using namespace std;

        template<typename T>
        concept has_increase_cpo = requires(T t, const size_t i)
        {
            increase(t, i);
        };

        template<typename T> // clang-format off
        constexpr auto invoke_increase_cpo(T&& v, const size_t i)
            noexcept(noexcept(increase(forward<T>(v), i))) // clang-format on
        {
            return increase(forward<T>(v), i);
        }

        template<typename T>
        concept has_decrease_cpo = requires(T t, const size_t i)
        {
            decrease(t, i);
        };

        template<typename T> // clang-format off
        constexpr auto invoke_decrease_cpo(T&& v, const size_t i)
            noexcept(noexcept(decrease(forward<T>(v), i))) // clang-format on
        {
            return decrease(forward<T>(v), i);
        }
    }

    struct increase
    {
        template<details::has_increase_cpo T> // clang-format off
        [[nodiscard]] constexpr auto operator()(T v, const std::size_t i) const noexcept(
            noexcept(details::invoke_increase_cpo(std::move(v), i))
        ) // clang-format on
        {
            return details::invoke_increase_cpo(std::move(v), i);
        }

        template<typename T>
            requires(!details::has_increase_cpo<T> && std::invocable<std::plus<>, T, std::size_t>)
        // clang-format off
        [[nodiscard]] constexpr auto operator()(T v, const std::size_t i) const noexcept(
            nothrow_invocable<std::plus<>, T, std::size_t>
        ) // clang-format on
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
        template<details::has_decrease_cpo T> // clang-format off
        [[nodiscard]] constexpr auto operator()(T v, const std::size_t i) const noexcept(
            noexcept(details::invoke_decrease_cpo(std::move(v), i))
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

    inline constexpr increase increase_v{};
    inline constexpr decrease decrease_v{};

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
            requires details::has_advance_cpo<T, Distance> // clang-format off
        [[nodiscard]] constexpr auto operator()(T& v, const Distance& distance) const noexcept(
             noexcept(details::invoke_advance_cpo(v, distance))
        ) // clang-format on
        {
            return details::invoke_advance_cpo(v, distance);
        }

        template<typename T, typename Distance> // clang-format off
            requires (
                !details::has_advance_cpo<T, Distance> &&
                std::invocable<std::plus<>, T, Distance>
            )
        [[nodiscard]] constexpr auto operator()(T& v, const Distance& distance) const noexcept(
            noexcept(plus_assign(v, distance))
        ) // clang-format on
        {
            return plus_assign(v, distance);
        }

        template<typename T, typename Distance> // clang-format off
        [[nodiscard]] constexpr auto operator()(T& v, const Distance& distance) const noexcept(
            noexcept(increase_v(v, distance), decrease_v(v, distance))
        ) // clang-format on
        {
            if(distance > 0) return increase_v(v, distance);
            return decrease_v(v, distance);
        }
    };

    inline constexpr advance advance_v{};

    template<std::invocable... Func> // clang-format off
    [[nodiscard]] constexpr auto merge_invoke(Func&&... func)
        noexcept(noexcept(std::tuple{std::invoke(std::forward<Func>(func))...})) // clang-format on
    {
        return std::tuple{std::invoke(std::forward<Func>(func))...};
    }

    template<std::invocable... Func> // clang-format off
        requires(std::same_as<std::invoke_result_t<Func>, void> || ...)
    [[nodiscard]] constexpr auto merge_invoke(Func&&... func)
        noexcept((std::is_nothrow_invocable_v<Func> && ...)) // clang-format on
    {
        (std::invoke(std::forward<Func>(func)), ...);
    }

    // c++23 feature
    template<typename ReturnT, typename Func, typename... Args>
        requires std::invocable<Func, Args...> &&
            std::convertible_to<std::invoke_result_t<Func, Args...>, ReturnT> // clang-format off
    [[nodiscard]] constexpr ReturnT invoke_r(Func&& func, Args&&... args)
        noexcept(std::is_nothrow_invocable_r_v<ReturnT, Func, Args...>) // clang-format on
    {
        return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    }

    namespace details
    {
        using namespace std;

        struct clone_fn
        {
            template<typename T>
                requires convertible_to<T, remove_cvref_t<T>>
            [[nodiscard]] constexpr remove_cvref_t<T> operator()(T&& t) const
                noexcept(is_nothrow_convertible_v<T, remove_cvref_t<T>>)
            {
                return forward<T>(t);
            }
        };
    }

    inline constexpr details::clone_fn clone{};
}
