#pragma once

#include <algorithm>

#ifdef NDEBUG
    #define INVALID_ARGUMENT void
#else
    #include <stdexcept>
    #define INVALID_ARGUMENT ::std::invalid_argument
#endif

#include "../functional/operations.h"
#include "../functional/invoke.h"

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
        template<
            typename T,
            typename Proj = ::std::identity,
            ::std::indirect_strict_weak_order<::std::projected<const T*, Proj>> Compare =
                ::std::ranges::less // clang-format off
        > // clang-format on
        [[nodiscard]] constexpr auto operator()( // NOLINTBEGIN(*-easily-swappable-parameters)
            const T& t,
            const T& min,
            const T& max,
            Compare cmp = {},
            Proj proj = {}
        ) const // NOLINTEND(*-easily-swappable-parameters)
#ifndef NDEBUG
            noexcept( //
                nothrow_predicate<
                    Compare,
                    ::std::projected<const T*, Proj>,
                    ::std::projected<const T*, Proj> // clang-format off
                > // clang-format on
            )
#endif
        {
            const auto& proj_max = ::std::invoke(proj, max);
            const auto& proj_min = ::std::invoke(proj, min);
            const auto& proj_t = ::std::invoke(proj, t);

            debug_throw<INVALID_ARGUMENT>(
                [&] { return invoke_r<bool>(cmp, proj_max, proj_min); },
                "max value should not less than min value"
            );

            return !invoke_r<bool>(cmp, proj_t, proj_min) && !invoke_r<bool>(cmp, proj_max, proj_t);
        }
    } is_between{};
}

#undef INVALID_ARGUMENT