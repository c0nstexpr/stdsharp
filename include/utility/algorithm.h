#pragma once

#include <algorithm>
#include <stdexcept>

#include <fmt/format.h>

#include "utility/assert.h"
#include "functional.h"

namespace blurringshadow::utility
{
    // clang-format off
    inline constexpr auto set_if = []<typename T, typename U, ::std::predicate<T, U> Comp>(
        T& left,
        U&& right,
        Comp comp = {}
    ) noexcept(
        ::blurringshadow::utility::nothrow_invocable_r<bool, Comp, U&, T&> &&
        ::blurringshadow::utility::nothrow_assignable_from<T, U>
    ) -> T&
    {
        // clang-format on
        if(::std::invoke(::std::move(comp), right, left)) left = ::std::forward<U>(right);
        return left;
    };

    inline constexpr auto set_if_greater = []<typename T, typename U>(T& left, U&& right) //
        noexcept( //
            noexcept( //
                ::blurringshadow::utility::set_if(
                    left,
                    ::std::forward<U>(right),
                    ::blurringshadow::utility::greater_v // clang-format off
                )
            )
        ) -> T& // clang-format on
    {
        return ::blurringshadow::utility::set_if(
            left, ::std::forward<U>(right), ::blurringshadow::utility::greater_v //
        ); //
    };

    inline constexpr auto set_if_less = []<typename T, typename U>(T& left, U&& right) //
        noexcept( //
            noexcept( //
                ::blurringshadow::utility::set_if(
                    left,
                    ::std::forward<U>(right),
                    ::blurringshadow::utility::less_v // clang-format off
                )
            )
        ) -> T& // clang-format on
    {
        return ::blurringshadow::utility::set_if(left, ::std::forward<U>(right), less_v); //
    };

    namespace details
    {
        struct is_between_fn
        {
            template<
                typename T,
                typename Min,
                typename Max,
                typename Compare,
                typename Proj // clang-format off
            > requires ::blurringshadow::utility::invocable_rnonvoid<Proj, const T> &&
                ::blurringshadow::utility::invocable_rnonvoid<Proj, const T> &&
                ::blurringshadow::utility::invocable_rnonvoid<Proj, const T> // clang-format on
            struct require
            {
                using proj_t = ::std::invoke_result_t<Proj, const T>;
                using proj_min = ::std::invoke_result_t<Proj, const Min>;
                using proj_max = ::std::invoke_result_t<Proj, const Max>;

                static constexpr auto value = ::std::predicate<Compare, proj_t, proj_min> && //
                    ::std::predicate<Compare, proj_max, proj_t> && //
                    ::std::predicate<Compare, proj_max, proj_min>;

                static constexpr auto nothrow_value =
                    ::blurringshadow::utility::nothrow_predicate<Compare, proj_t, proj_min> && //
                    ::blurringshadow::utility::nothrow_predicate<Compare, proj_max, proj_t> && //
                    ::blurringshadow::utility::nothrow_predicate<Compare, proj_max, proj_min>;
            };

            template<
                typename T,
                typename U,
                typename V,
                typename Compare = ::std::ranges::less,
                typename Proj = ::std::identity // clang-format off
            > requires ::blurringshadow::utility::details::is_between_fn::require<T, U, V, Compare, Proj>::value
            [[nodiscard]] constexpr bool operator()(
                const T& v,
                const U& min,
                const V& max,
                Compare cmp = {},
                Proj proj = {}
            ) const noexcept(!is_debug && require<T, U, V, Compare, Proj>::nothrow_value) // clang-format on
            {
                using traits = require<T, U, V, Compare, Proj>;

                const auto& projected_v = ::std::invoke(proj, v);
                const auto& projected_min = ::std::invoke(proj, min);
                const auto& projected_max = ::std::invoke(proj, max);

                if constexpr(is_debug)
                    if(::blurringshadow::utility::invoke_r<bool>(cmp, projected_max, projected_min))
                    {
                        if constexpr(
                            ::fmt::formattable<typename traits::proj_min>::value &&
                            ::fmt::formattable<typename traits::proj_max>::value // clang-format off
                        ) throw ::std::invalid_argument{
                            ::fmt::format(
                                "projected max value {} " 
                                "should not less than projected min value {}",
                                projected_max,
                                projected_min
                            )
                        };
                        else throw ::std::invalid_argument{
                            "projected max value should not less than projected min value"
                        }; // clang-format on
                    } // clang-format off

                return !::blurringshadow::utility::invoke_r<bool>(cmp, projected_v, projected_min) &&
                    !::blurringshadow::utility::invoke_r<bool>(cmp, projected_max, projected_v);
            } // clang-format on
        };
    }

    inline constexpr details::is_between_fn is_between{};
}
