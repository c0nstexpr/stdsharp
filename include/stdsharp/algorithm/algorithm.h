#pragma once

#include <algorithm>

#include "../ranges/ranges.h"
#include "../cassert/cassert.h"
#include "../functional/operations.h"
#include "../compare/compare.h"

namespace stdsharp
{
    inline constexpr auto set_if = []<typename T, typename U, std::predicate<U, T> Comp>
        requires std::assignable_from<T&, U> // clang-format off
        (T& left, U&& right, Comp comp = {})
        noexcept(nothrow_predicate<Comp, U, T> && nothrow_assignable_from<T&, U>)
        -> T& // clang-format on
    {
        if(std::invoke(cpp_move(comp), right, left)) left = cpp_forward(right);
        return left;
    };

    using set_if_fn = decltype(set_if);

    inline constexpr auto set_if_greater = []<typename T, typename U>
        requires std::invocable<set_if_fn, T&, U, std::ranges::greater> // clang-format off
        (T & left, U && right)
        noexcept(nothrow_invocable<set_if_fn, T&, U, std::ranges::greater>) -> T& // clang-format on
    { return set_if(left, cpp_forward(right), greater_v); };

    using set_if_greater_fn = decltype(set_if_greater);

    inline constexpr auto set_if_less = []<typename T, typename U>
        requires std::invocable<set_if_fn, T&, U, std::ranges::less> // clang-format off
        (T& left, U&& right)
        noexcept(nothrow_invocable<set_if_fn, T&, U, std::ranges::less>) -> T& // clang-format on
    { return set_if(left, cpp_forward(right), less_v); };

    using set_if_less_fn = decltype(set_if_less);

    inline constexpr struct is_between_fn
    {
        template<
            typename T,
            typename Proj = std::identity,
            std::indirect_strict_weak_order<std::projected<const T*, Proj>> Compare =
                std::ranges::less // clang-format off
        > // clang-format on
        [[nodiscard]] constexpr auto operator()( // NOLINTBEGIN(*-easily-swappable-parameters)
            const T& t,
            const T& min,
            const T& max,
            Compare cmp = {},
            Proj proj = {}
        ) const // NOLINTEND(*-easily-swappable-parameters)
            noexcept( //
                nothrow_predicate<
                    Compare,
                    std::projected<const T*, Proj>,
                    std::projected<const T*, Proj> // clang-format off
                > // clang-format on
            )
        {
            const auto& proj_max = std::invoke(proj, max);
            const auto& proj_min = std::invoke(proj, min);
            const auto& proj_t = std::invoke(proj, t);

            Expects(!std::invoke(cmp, proj_max, proj_min));

            return !std::invoke(cmp, proj_t, proj_min) && !std::invoke(cmp, proj_max, proj_t);
        }
    } is_between{};

    constexpr struct strict_compare_fn
    {
        template<std::ranges::input_range TRng, std::ranges::input_range URng>
            requires std::three_way_comparable_with<
                range_const_reference_t<TRng>,
                range_const_reference_t<URng>>
        constexpr auto operator()(const TRng& left, const URng& right) const
        {
            using ordering = std::partial_ordering;

            auto pre = ordering::equivalent;
            const auto cmp_impl = [](ordering& pre, const ordering next)
            {
                if(is_eq(pre))
                {
                    pre = next;
                    return;
                }

                if(pre != next && is_neq(next))
                {
                    pre = ordering::unordered;
                    return;
                }
            };

            {
                auto l_it = std::ranges::cbegin(left);
                auto r_it = std::ranges::cbegin(right);
                const auto l_end = std::ranges::cend(left);
                const auto r_end = std::ranges::cend(right);
                for(; !is_ud(pre); ++l_it, ++r_it)
                {
                    if(l_it == l_end)
                    {
                        if(r_it != r_end) cmp_impl(pre, ordering::less);
                        break;
                    }

                    if(r_it == r_end)
                    {
                        cmp_impl(pre, ordering::greater);
                        break;
                    }

                    cmp_impl(pre, std::compare_three_way{}(*l_it, *r_it));
                }
            }

            return pre;
        }
    } strict_compare{};
}
