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
            ::stdsharp::concepts::nothrow_invocable_r<Comp, bool, U, T> &&
            ::stdsharp::concepts::nothrow_assignable_from<T, U&&>
        ) ->T& // clang-format on
    {
        if(::stdsharp::functional::invoke_r<bool>(::std::move(comp), right, left))
            left = ::std::forward<U>(right);
        return left;
    };

    inline constexpr auto set_if_greater = []<typename T, typename U>
        requires ::std::invocable<
            decltype(::stdsharp::algorithm::set_if),
            T&,
            U,
            ::std::ranges::greater // clang-format off
        >
        (T& left, U&& right)
        noexcept(
            noexcept(
                ::stdsharp::algorithm::set_if(
                    left,
                    ::std::forward<U>(right),
                    ::stdsharp::functional::greater_v
                )
            )
        ) -> T& // clang-format on
    {
        return ::stdsharp::algorithm::set_if(
            left, ::std::forward<U>(right), ::stdsharp::functional::greater_v //
        ); //
    };

    inline constexpr auto set_if_less = []<typename T, typename U>
        requires ::std::invocable<
            decltype(::stdsharp::algorithm::set_if),
            T&,
            U,
            ::std::ranges::less // clang-format off
        >
        (T& left, U&& right)
        noexcept( //
            noexcept( //
                ::stdsharp::algorithm::set_if(
                    left,
                    ::std::forward<U>(right),
                    ::stdsharp::functional::less_v
                )
            )
        ) -> T& // clang-format on
    {
        return ::stdsharp::algorithm::set_if(
            left, ::std::forward<U>(right), ::stdsharp::functional::less_v //
        );
    };

    namespace details
    {
        struct is_between_fn
        {
            template<
                typename T,
                typename Min,
                typename Max,
                typename Proj // clang-format off
            > // clang-format on
            struct require
            {
                using proj_t = ::std::invoke_result_t<Proj, T>;
                using proj_min = ::std::invoke_result_t<Proj, Min>;
                using proj_max = ::std::invoke_result_t<Proj, Max>;

                template<typename Compare>
                    requires ::std::predicate<Compare, const proj_t, const proj_min> &&
                        ::std::predicate<Compare, const proj_max, const proj_t> &&
                        ::std::predicate<Compare, const proj_max, const proj_min>
                static constexpr bool nothrow_v = !::stdsharp::cassert::is_debug &&
                    ::stdsharp::concepts::
                        nothrow_predicate<Compare, const proj_t, const proj_min> && //
                    ::stdsharp::concepts::
                        nothrow_predicate<Compare, const proj_max, const proj_t> && //
                    ::stdsharp::concepts::
                        nothrow_predicate<Compare, const proj_max, const proj_min>;
            };
        };
    }

    inline constexpr ::stdsharp::functional::invocable_obj is_between(
        ::stdsharp::functional::nodiscard_tag,
        []< // clang-format on
            typename T,
            typename U,
            typename V,
            typename Compare = ::std::ranges::less,
            typename Proj = ::std::identity,
            auto noexcept_ = ::stdsharp::algorithm::details:: // clang-format off
            is_between_fn::require<T, U, V, Proj>::template nothrow_v<Compare>
        >( // clang-format on
            T&& v,
            U&& min,
            V&& max,
            Compare cmp = {},
            Proj proj = {} // clang-format off
        ) noexcept(noexcept_) // clang-format on
        {
            using is_between_traits =
                ::stdsharp::algorithm::details::is_between_fn::require<T, U, V, Proj>;

            const auto& projected_v = ::std::invoke(proj, ::std::forward<T>(v));
            const auto& projected_min = ::std::invoke(proj, ::std::forward<U>(min));
            const auto& projected_max = ::std::invoke(proj, ::std::forward<V>(max));

            if constexpr(::stdsharp::cassert::is_debug)
                if(::stdsharp::functional::invoke_r<bool>(cmp, projected_max, projected_min))
                {
                    if constexpr(
                        ::fmt::is_formattable<typename is_between_traits::proj_min>::value &&
                        ::fmt::is_formattable<
                            typename is_between_traits::proj_max>::value // clang-format off
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
                }

            return !::stdsharp::functional::invoke_r<bool>(cmp, projected_v, projected_min) &&
                !::stdsharp::functional::invoke_r<bool>(cmp, projected_max, projected_v);
        } //
    );
}
