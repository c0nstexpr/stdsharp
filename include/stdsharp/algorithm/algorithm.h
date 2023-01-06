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

    namespace details
    {
        template<typename Compare, typename T, typename Min, typename Max>
        concept min_max_predicate = ::std::predicate<Compare, const T&, const Min&> &&
            ::std::predicate<Compare, const Max&, const T&> &&
            ::std::predicate<Compare, const Max&, const Min&>;

        template<typename Compare, typename T, typename Min, typename Max>
        concept nothrow_min_max_predicate = nothrow_predicate<Compare, const T&, const Min&> &&
            nothrow_predicate<Compare, const Max&, const T&> &&
            nothrow_predicate<Compare, const Max&, const Min&>;
    }

    inline constexpr struct clamp_fn
    {
        template<
            typename T,
            typename Min,
            typename Max,
            details::min_max_predicate<T, Min, Max> Compare =
                ::std::ranges::less // clang-format off
        > // clang-format on
        [[nodiscard]] constexpr ::std::common_reference_t<const T&, const Min&, const Max&>
            operator()(const T& t, const Min& min, const Max& max, Compare cmp = {}) const
            noexcept(details::nothrow_min_max_predicate<Compare, T, Min, Max> && !is_debug)
        {
            if constexpr(is_debug)
                if(invoke_r<bool>(cmp, max, min))
                    throw ::std::invalid_argument{"max value should not less than min value"};

            return invoke_r<bool>(cmp, t, min) ? min : (invoke_r<bool>(cmp, max, t) ? max : t);
        }
    } clamp{};

    inline constexpr struct is_between_fn
    {
        template<
            typename T,
            typename Min,
            typename Max,
            details::min_max_predicate<T, Min, Max> Compare =
                ::std::ranges::less // clang-format off
        > // clang-format on
        [[nodiscard]] constexpr auto
            operator()(const T& t, const Min& min, const Max& max, Compare&& cmp = {}) const
            noexcept(details::nothrow_min_max_predicate<Compare, T, Min, Max> && !is_debug)
        {
            if constexpr(is_debug)
                if(invoke_r<bool>(cmp, max, min))
                    throw ::std::invalid_argument{"max value should not less than min value"};

            return !invoke_r<bool>(cmp, t, min) && !invoke_r<bool>(cmp, max, t);
        }
    } is_between{};
}