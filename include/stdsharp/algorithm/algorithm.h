#pragma once

#include <algorithm>

#ifndef NDEBUG
    #include <stdexcept>
#endif

#include "../functional/operations.h"
#include "../functional/invoke.h"
#include "../cassert/cassert.h"

namespace stdsharp
{
    inline constexpr auto set_if = []<typename T, typename U, ::std::predicate<U, T> Comp>
        requires ::std::assignable_from<T&, U> // clang-format off
        (T& left, U&& right, Comp comp = {})
        noexcept(nothrow_predicate<Comp, U, T> && nothrow_assignable_from<T&, U>)
        -> T& // clang-format on
    {
        if(invoke_r<bool>(::std::move(comp), right, left)) left = ::std::forward<U>(right);
        return left;
    };

    using set_if_fn = decltype(set_if);

    inline constexpr auto set_if_greater = []<typename T, typename U>
        requires ::std::invocable<set_if_fn, T&, U, ::std::ranges::greater> // clang-format off
        (T & left, U && right)
        noexcept(nothrow_invocable<set_if_fn, T&, U, ::std::ranges::greater>) -> T& // clang-format on
    {
        return set_if(left, ::std::forward<U>(right), greater_v);
    };

    using set_if_greater_fn = decltype(set_if_greater);

    inline constexpr auto set_if_less = []<typename T, typename U>
        requires ::std::invocable<set_if_fn, T&, U, ::std::ranges::less> // clang-format off
        (T& left, U&& right)
        noexcept(nothrow_invocable<set_if_fn, T&, U, ::std::ranges::less>) -> T& // clang-format on
    {
        return set_if(left, ::std::forward<U>(right), less_v);
    };

    using set_if_less_fn = decltype(set_if_less);

    inline constexpr struct is_between_fn
    {
        template<typename T, typename Min, typename Max, typename Compare = ::std::ranges::less>
            requires ::std::predicate<Compare, const T, const Min> &&
            ::std::predicate<Compare, const Max, const T> &&
            ::std::predicate<Compare, const Max, const Min>
        [[nodiscard]] constexpr auto
            operator()(const T& t, const Min& min, const Max& max, Compare cmp = {}) const noexcept(
                nothrow_predicate<Compare, const T, const Min>&&
                    nothrow_predicate<Compare, const Max, const T> &&
                !is_debug
            )
        {
            if constexpr(is_debug)
                if(invoke_r<bool>(cmp, max, min))
                    throw ::std::invalid_argument{"max value should not less than min value"};

            return !invoke_r<bool>(cmp, t, min) && !invoke_r<bool>(cmp, max, t);
        }
    } is_between{};
}