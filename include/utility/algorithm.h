#pragma once

#include <algorithm>
#include <stdexcept>

#include <fmt/format.h>

#include "utility/assert.h"
#include "functional.h"

namespace blurringshadow::utility
{
    // clang-format off
    inline constexpr auto set_if = []<typename T, typename U, std::predicate<T, U> Comp>(
        T& left,
        U&& right,
        Comp comp = {}
    ) noexcept(nothrow_invocable_r<bool, Comp, U&, T&> && nothrow_assignable_from<T, U>)  -> T& 
    {
        // clang-format on
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
        using namespace std;

        struct is_between_fn
        {
            template<
                typename T,
                typename Min,
                typename Max,
                typename Compare,
                typename Proj // clang-format off
            > requires invocable_rnonvoid<Proj, const T> &&
                invocable_rnonvoid<Proj, const T> &&
                invocable_rnonvoid<Proj, const T> // clang-format on
            struct require
            {
                using proj_t = std::invoke_result_t<Proj, const T>;
                using proj_min = std::invoke_result_t<Proj, const Min>;
                using proj_max = std::invoke_result_t<Proj, const Max>;

                static constexpr auto value = predicate<Compare, proj_t, proj_min> && //
                    predicate<Compare, proj_max, proj_t> && //
                    predicate<Compare, proj_max, proj_min>;
            };

            template<
                typename T,
                typename U,
                typename V,
                typename Compare = ranges::less,
                typename Proj = std::identity // clang-format off
            > requires require<T, U, V, Compare, Proj>::value
            [[nodiscard]] constexpr bool operator()(
                const T& v,
                const U& min,
                const V& max,
                Compare cmp = {},
                Proj proj = {}
            ) const // clang-format on
            {
                using namespace std;
                using traits = require<T, U, V, Compare, Proj>;

                const auto& projected_v = invoke(proj, v);
                const auto& projected_min = invoke(proj, min);
                const auto& projected_max = invoke(proj, max);

                if constexpr(is_debug)
                    if(invoke_r<bool>(cmp, projected_max, projected_min))
                    {
                        using namespace fmt;

                        if constexpr(
                            formattable<typename traits::proj_min>::value &&
                            formattable<typename traits::proj_max>::value // clang-format off
                        ) throw invalid_argument{
                            format(
                                "projected max value {} " 
                                "should not less than projected min value {}",
                                projected_max,
                                projected_min
                            )
                        };
                        else throw invalid_argument{
                            "projected max value should not less than projected min value"
                        }; // clang-format on
                    }

                return !invoke_r<bool>(cmp, projected_v, projected_min) &&
                    !invoke_r<bool>(cmp, projected_max, projected_v);
            }
        };
    }

    inline constexpr details::is_between_fn is_between{};
}
