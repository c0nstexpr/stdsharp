#pragma once

#include <algorithm>

#ifndef NDEBUG
    #include <fmt/format.h>
    #include <stdexcept>
#endif

#include "functional/operations.h"
#include "functional/invoke.h"
#include "cassert/cassert.h"

namespace stdsharp
{
    inline constexpr auto set_if = []<typename T, typename U, ::std::predicate<U, T> Comp>
        requires ::std::assignable_from<T&, U> // clang-format off
        (
            T& left,
            U&& right,
            Comp comp = {}
        ) noexcept(
            concepts::nothrow_predicate<Comp, U, T> &&
            concepts::nothrow_assignable_from<T&, U>
        ) ->T& // clang-format on
    {
        if(functional::invoke_r<bool>(::std::move(comp), right, left))
            left = ::std::forward<U>(right);
        return left;
    };

    using set_if_fn = decltype(set_if);

    inline constexpr auto set_if_greater = []<typename T, typename U>
        requires ::std::invocable<set_if_fn, T&, U, ::std::ranges::greater>
            // clang-format off
        (T& left, U&& right) // clang-format on
    noexcept(concepts::nothrow_invocable<set_if_fn, T&, U, ::std::ranges::greater>)->T&
    {
        return set_if(left, ::std::forward<U>(right), functional::greater_v);
    };

    using set_if_greater_fn = decltype(set_if_greater);

    inline constexpr auto set_if_less = []<typename T, typename U>
        requires ::std::invocable<set_if_fn, T&, U, ::std::ranges::less>
            // clang-format off
    (T& left, U&& right) // clang-format on
    noexcept(concepts::nothrow_invocable<set_if_fn, T&, U, ::std::ranges::less>)->T&
    {
        return set_if(left, ::std::forward<U>(right), functional::less_v);
    };

    using set_if_less_fn = decltype(set_if_less);

    inline constexpr struct is_between_fn
    {
        template<
            typename T,
            typename Min,
            typename Max,
            typename Compare = ::std::ranges::less // clang-format off
        > // clang-format on
            requires ::std::predicate<Compare, const T, const Min> &&
                ::std::predicate<Compare, const Max, const T> &&
                ::std::predicate<Compare, const Max, const Min>
        [[nodiscard]] constexpr auto operator()(
            const T& v, //
            const Min& min,
            const Max& max,
            Compare cmp = {} //
        ) const
            noexcept(
                !cassert::is_debug && concepts::nothrow_predicate<Compare, const T, const Min> &&
                concepts::nothrow_predicate<Compare, const Max, const T> &&
                concepts::nothrow_predicate<Compare, const Max, const Min> // clang-format off
        ) // clang-format on
        {
            if constexpr(cassert::is_debug)
                if(functional::invoke_r<bool>(cmp, max, min))
                {
                    if constexpr(
                        ::fmt::is_formattable<Min>::value && ::fmt::is_formattable<Max>::value //
                    )
                        throw ::std::invalid_argument{// clang-format off
                            ::fmt::format(
                                "projected max value {} should not less than projected min value {}",
                                max,
                                min
                            )
                        };
                    else throw ::std::invalid_argument{
                        "projected max value should not less than projected min value"
                    }; // clang-format on
                }

            return !functional::invoke_r<bool>(cmp, v, min) &&
                !functional::invoke_r<bool>(cmp, max, v);
        }

        template<
            typename T,
            typename Min,
            typename Max,
            typename Compare,
            typename Proj // clang-format off
        > // clang-format on
            requires ::std::invocable<Proj, const T> && //
                ::std::invocable<Proj, const Min> && //
                ::std::invocable<Proj, const Max> && //
                ::std::invocable<
                    is_between_fn,
                    ::std::invoke_result_t<Proj, const T>,
                    ::std::invoke_result_t<Proj, const Min>,
                    ::std::invoke_result_t<Proj, const Max>,
                    Compare // clang-format off
                > // clang-format on
        [[nodiscard]] constexpr auto operator()(
            const T& v, //
            const Min& min,
            const Max& max,
            Compare&& cmp,
            Proj proj //
        ) const noexcept( //
            concepts::nothrow_invocable<Proj, const T>&& // clang-format off
                concepts::nothrow_invocable<Proj, const Min> &&
                concepts::nothrow_invocable<Proj, const Max> &&
                concepts::nothrow_invocable<
                    is_between_fn,
                    ::std::invoke_result_t<Proj, const T>,
                    ::std::invoke_result_t<Proj, const Min>,
                    ::std::invoke_result_t<Proj, const Max>,
                    Compare
                > // clang-format on
        )
        {
            return (*this)(
                ::std::invoke(proj, v),
                ::std::invoke(proj, min),
                ::std::invoke(proj, max),
                ::std::forward<Compare>(cmp) //
            );
        }
    } is_between{};
}
