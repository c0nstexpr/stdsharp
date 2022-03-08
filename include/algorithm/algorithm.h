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
    private:
        struct check_fn
        {
            template<typename Min, typename Max, typename Compare>
                requires(is_debug)
            constexpr auto operator()(const Min& min, const Max& max, Compare& cmp) const
            {
                if(functional::invoke_r<bool>(cmp, max, min))
                {
                    if(::std::is_constant_evaluated())
                    {
                        static_assert(true, "max value should not less than min value");
                        return;
                    }

                    if constexpr(
                        ::fmt::is_formattable<Min>::value && //
                        ::fmt::is_formattable<Max>::value //
                    )
                        throw ::std::invalid_argument{
                            // clang-format off
                            ::fmt::format(
                                "max value {} should not less than min value {}",
                                max,
                                min
                            )
                        }; // clang-format on
                    else
                        throw ::std::invalid_argument{"max value should not less than min value"};
                }
            }
        };

    public:
        template<typename T, typename Min, typename Max, typename Compare = ::std::ranges::less>
            requires ::std::predicate<Compare, const T, const Min> &&
                ::std::predicate<Compare, const Max, const T> &&
                ::std::predicate<Compare, const Max, const Min>
        [[nodiscard]] constexpr auto
            operator()(const T& t, const Min& min, const Max& max, Compare cmp = {}) const
            noexcept( // clang-format off
                concepts::nothrow_predicate<Compare, const T, const Min> &&
                    concepts::nothrow_predicate<Compare, const Max, const T> &&
                    functional::nothrow_optional_invocable<check_fn, Max, Min, Compare>
            ) // clang-format on
        {
            functional::optional_invoke(check_fn{}, max, min, cmp);

            return !functional::invoke_r<bool>(cmp, t, min) &&
                !functional::invoke_r<bool>(cmp, max, t);
        }
    } is_between{};
}
