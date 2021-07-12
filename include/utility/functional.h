#pragma once

#include <functional>

#include "type_traits.h"

namespace blurringshadow::utility
{
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

        template<bool Requirement> // clang-format off
        inline constexpr auto operator_assign = []<
            typename T,
            typename U,
            typename Operation,
            typename AlternativeOperation
        >(
            T& left,
            U&& right,
            Operation op,
            AlternativeOperation al_op
        ) noexcept(
            Requirement && nothrow_invocable<Operation, T&, U&&> ||
            nothrow_invocable<AlternativeOperation, T&, U&&> && 
            nothrow_assignable_from<T, invoke_result_t<AlternativeOperation, T&, U&&>>
        ) // clang-format on
        {
            if constexpr(Requirement) return std::invoke(op, left, forward<U>(right));
            else
            {
                left = std::invoke(al_op, left, forward<U>(right));
                return left;
            }
        };
    }

#define BS_UTIL_ASSIGN_OPERATE(operator_type, op)                                         \
    namespace details                                                                     \
    {                                                                                     \
        template<typename T, typename U>                                                  \
        concept operator_type##_assignable = requires(T a, U b)                           \
        {                                                                                 \
            a op## = b;                                                                   \
        };                                                                                \
                                                                                          \
        struct operator_type##_assign_fn                                                  \
        {                                                                                 \
            template<typename T, typename U>                                              \
            static constexpr auto operator_assign_fn =                                    \
                operator_assign<operator_type##_assignable<T, U>>;                        \
                                                                                          \
            template<typename T, typename U>                                              \
            [[nodiscard]] constexpr auto operator()(T& left, U&& right) const             \
                noexcept(noexcept(operator_assign_fn<T, U>(left, forward<U>(right))))     \
            {                                                                             \
                return operator_assign_fn<T, U>(                                          \
                    left,                                                                 \
                    forward<U>(right),                                                    \
                    []<typename FwU>(T& l, FwU&& r) { return l op## = forward<FwU>(r); }, \
                    operator_type##_v /**/                                                \
                );                                                                        \
            }                                                                             \
        };                                                                                \
    }                                                                                     \
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

    inline constexpr std::identity identity_v{};

    namespace details
    {
        using namespace std;

        template<typename T>
        concept has_increase_cpo = requires(T v, const size_t i)
        {
            increase(v, i);
        };

        template<has_increase_cpo T>
        constexpr auto invoke_increase_cpo(T&& v, const size_t i) //
            noexcept(increase(forward<T>(v), i))
        {
            return increase(forward<T>(v), i);
        }


        template<typename T>
        concept has_decrease_cpo = requires(T v, const size_t i)
        {
            decrease(v, i);
        };

        template<has_decrease_cpo T>
        constexpr auto invoke_decrease_cpo(T&& v, const size_t i) //
            noexcept(noexcept(decrease(forward<T>(v), i)))
        {
            return decrease(forward<T>(v), i);
        }
    }

    struct increase
    {
        template<details::has_increase_cpo T>
        [[nodiscard]] constexpr auto operator()(T&& v, const std::size_t i) const
            noexcept(noexcept(details::invoke_increase_cpo(std::forward<T>(v), i)))
        {
            return details::invoke_increase_cpo(std::forward<T>(v), i);
        }

        template<typename T>
            requires(!details::has_increase_cpo<T> && std::invocable<std::plus<>, T, std::size_t>)
        [[nodiscard]] constexpr auto operator()(T&& v, const std::size_t i) const
            noexcept(nothrow_invocable<std::plus<>, T&&, std::size_t>)
        {
            return plus_v(std::forward<T>(v), i);
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
        [[nodiscard]] constexpr auto operator()(T&& v, const std::size_t i) const
            noexcept(noexcept(details::invoke_decrease_cpo(std::forward<T>(v), i)))
        {
            return details::invoke_decrease_cpo(std::forward<T>(v), i);
        }

        template<typename T>
            requires(!details::has_decrease_cpo<T> && std::invocable<std::minus<>, T, std::size_t>)
        [[nodiscard]] constexpr auto operator()(T&& v, const std::size_t i) const
            noexcept(nothrow_invocable<std::minus<>, T, std::size_t>)
        {
            return minus_v(std::forward<T>(v), i);
        }

        template<typename T>
        [[nodiscard]] constexpr auto operator()(T v, std::size_t i) const noexcept(noexcept(--v))
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
        constexpr auto invoke_advance_cpo(T& t, const Distance& d) noexcept(noexcept(advance(t, d)))
        {
            return advance(t, d);
        }

        template<typename T, typename Distance>
        concept has_advance_cpo = requires(T& t, const Distance& d)
        {
            invoke_advance_cpo(t, d);
        };
    }

    struct advance
    {
        template<typename T, typename Distance>
            requires details::has_advance_cpo<T, Distance>
        [[nodiscard]] constexpr auto operator()(T& v, const Distance& distance) const
            noexcept(noexcept(details::invoke_advance_cpo(v, distance)))
        {
            return details::invoke_advance_cpo(v, distance);
        }

        template<typename T, typename Distance>
            requires( //
                !details::has_advance_cpo<T, Distance> &&
                std::invocable<std::plus<>, T, Distance> //
            )
        [[nodiscard]] constexpr auto operator()(T& v, const Distance& distance) const
            noexcept(noexcept(plus_assign(v, distance)))
        {
            return plus_assign(v, distance);
        }

        template<typename T, typename Distance>
        [[nodiscard]] constexpr auto operator()(T& v, const Distance& distance) const
            noexcept(noexcept(increase_v(v, distance), decrease_v(v, distance)))
        {
            if(distance > 0) return increase_v(v, distance);
            return decrease_v(v, distance);
        }
    };

    inline constexpr advance advance_v{};

    template<std::invocable... Func>
    [[nodiscard]] constexpr auto merge_invoke(Func&&... func) //
        noexcept(noexcept(std::tuple{std::invoke(std::forward<Func>(func))...}))
    {
        return std::tuple{std::invoke(std::forward<Func>(func))...};
    }

    template<std::invocable... Func>
        requires(std::same_as<std::invoke_result_t<Func>, void> || ...)
    [[nodiscard]] constexpr auto merge_invoke(Func&&... func) //
        noexcept((nothrow_invocable<Func&&> && ...))
    {
        (std::invoke(std::forward<Func>(func)), ...);
    }

    // c++23 feature
    template<typename ReturnT, typename Func, typename... Args>
        requires std::invocable<Func, Args...> &&
            std::convertible_to<std::invoke_result_t<Func, Args...>, ReturnT>
    [[nodiscard]] constexpr ReturnT invoke_r(Func&& func, Args&&... args) //
        noexcept(nothrow_invocable_r<ReturnT, Func, Args...>)
    {
        return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    }

    namespace details
    {
        struct clone_fn
        {
            template<typename T>
            [[nodiscard]] constexpr std::remove_cvref_t<T> operator()(T&& t) const 
                noexcept(nothrow_constructible_from<std::remove_cvref_t<T>, T>)
            {
                return forward<T>(t);
            }
        };
    }

    inline constexpr details::clone_fn clone{};
}
