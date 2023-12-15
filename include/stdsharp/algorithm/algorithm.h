#pragma once

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
        if(invoke(cpp_move(comp), right, left)) left = cpp_forward(right);
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
            const auto& proj_max = invoke(proj, max);
            const auto& proj_min = invoke(proj, min);
            const auto& proj_t = invoke(proj, t);

            Expects(!invoke(cmp, proj_max, proj_min));

            return !invoke(cmp, proj_t, proj_min) && !invoke(cmp, proj_max, proj_t);
        }
    } is_between{};

    inline constexpr struct strict_compare_fn
    {
    private:
        using ordering = std::partial_ordering;

        static constexpr auto cmp_impl(ordering& pre, const ordering next) noexcept
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

    public:
        template<
            std::input_iterator I1,
            std::sentinel_for<I1> S1,
            std::input_iterator I2,
            std::sentinel_for<I2> S2,
            typename Cmp = std::compare_three_way>
            requires ordering_predicate<Cmp&, std::iter_reference_t<I1>, std::iter_reference_t<I2>>
        constexpr auto operator()(I1 i1, const S1 s1, I2 i2, const S2 s2, Cmp cmp = {}) const
        {
            auto pre = ordering::equivalent;

            for(; !is_ud(pre); ++i1, ++i2)
            {
                if(i1 == s1)
                {
                    if(i2 != s2) pre = ordering::unordered;
                    break;
                }

                if(i2 == s2)
                {
                    pre = ordering::unordered;
                    break;
                }

                cmp_impl(pre, invoke(cmp, *i1, *i2));
            }

            return pre;
        }

        template<
            std::ranges::input_range R1,
            std::ranges::input_range R2,
            typename Cmp = std::compare_three_way>
            requires ordering_predicate<
                Cmp&,
                std::ranges::range_reference_t<R1>,
                std::ranges::range_reference_t<R2>>
        constexpr auto operator()(R1&& r1, R2&& r2, Cmp cmp = {}) const
        {
            return (*this)(
                std::ranges::begin(r1),
                std::ranges::end(r1),
                std::ranges::begin(r2),
                std::ranges::end(r2),
                cmp
            );
        }
    } strict_compare{};

    template<typename I, typename O>
    using move_n_result = std::ranges::in_out_result<I, O>;

    inline constexpr struct move_n_fn
    {
        template<std::input_iterator I, std::weakly_incrementable O>
            requires std::indirectly_movable<I, O>
        constexpr move_n_result<I, O>
            operator()(I in, const std::iter_difference_t<I> n, O out) const
        {
            auto&& r = std::ranges::move(
                std::counted_iterator{cpp_move(in), n},
                std::default_sentinel,
                cpp_move(out)
            );

            return {cpp_move(r.in.base()), cpp_move(r.out)};
        }
    } move_n{};
}
