#pragma once

#include <algorithm>
#include <stdexcept>

#ifndef NDEBUG
    #include <fmt/format.h>
#endif

#include "functional/functional.h"
#include "cassert/cassert.h"

namespace stdsharp::algorithm
{
    inline constexpr auto set_if = []<typename U, typename T, ::std::predicate<U, T> Comp>
        requires ::std::assignable_from<T&, U&&> // clang-format off
        (
            T& left,
            U&& right,
            Comp comp = {}
        ) noexcept(
            concepts::nothrow_invocable_r<Comp, bool, U, T> &&
            concepts::nothrow_assignable_from<T, U&&>
        ) ->T& // clang-format on
    {
        if(functional::invoke_r<bool>(::std::move(comp), right, left))
            left = ::std::forward<U>(right);
        return left;
    };

    inline constexpr auto set_if_greater = []<typename T, typename U>
        requires ::std::invocable<decltype(set_if), T&, U, ::std::ranges::greater>
            // clang-format off
    (T& left, U&& right) // clang-format on
    noexcept(concepts::nothrow_invocable<decltype(set_if), T&, U, ::std::ranges::greater>)->T&
    {
        return set_if(left, ::std::forward<U>(right), functional::greater_v);
    };

    inline constexpr auto set_if_less = []<typename T, typename U>
        requires ::std::invocable<decltype(set_if), T&, U, ::std::ranges::less>
            // clang-format off
    (T& left, U&& right) // clang-format on
    noexcept(concepts::nothrow_invocable<decltype(set_if), T&, U, ::std::ranges::less>)->T&
    {
        return set_if(left, ::std::forward<U>(right), functional::less_v);
    };

    inline constexpr functional::invocable_obj is_between(
        functional::nodiscard_tag,
        []< // clang-format on
            typename T,
            typename Min,
            typename Max,
            typename Compare = ::std::ranges::less,
            typename Proj = ::std::identity,
            typename ProjT = ::std::invoke_result_t<Proj, T>,
            typename ProjMin = ::std::invoke_result_t<Proj, Min>,
            typename ProjMax = ::std::invoke_result_t<Proj, Max> // clang-format off
        >
            requires ::std::predicate<Compare, const ProjT, const ProjMin> &&
                ::std::predicate<Compare, const ProjMax, const ProjT> &&
                ::std::predicate<Compare, const ProjMax, const ProjMin> // clang-format on
        (T&& v, Min&& min, Max&& max, Compare cmp = {}, Proj proj = {}) noexcept(
            !cassert::is_debug &&
            concepts::nothrow_predicate<Compare, const ProjT, const ProjMin> &&
            concepts::nothrow_predicate<Compare, const ProjMax, const ProjT> &&
            concepts::nothrow_predicate<Compare, const ProjMax, const ProjMin> // clang-format off
        ) // clang-format on
        {
            const auto& projected_v = ::std::invoke(proj, ::std::forward<T>(v));
            const auto& projected_min = ::std::invoke(proj, ::std::forward<Min>(min));
            const auto& projected_max = ::std::invoke(proj, ::std::forward<Max>(max));

            if constexpr(cassert::is_debug)
                if(functional::invoke_r<bool>(cmp, projected_max, projected_min))
                {
                    if constexpr(
                        ::fmt::is_formattable<ProjMin>::value &&
                        ::fmt::is_formattable<ProjMax>::value)
                        throw ::std::invalid_argument{// clang-format off
                            ::fmt::format(
                                "projected max value {} should not less than projected min value {}",
                                projected_max,
                                projected_min
                            )
                        };
                    else throw ::std::invalid_argument{
                        "projected max value should not less than projected min value"
                    }; // clang-format on
                }

            return !functional::invoke_r<bool>(cmp, projected_v, projected_min) &&
                !functional::invoke_r<bool>(cmp, projected_max, projected_v);
        } //
    );
}
