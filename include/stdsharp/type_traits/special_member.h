#pragma once

#include "../concepts/concepts.h"
#include "../type_traits/core_traits.h"

namespace stdsharp
{
    struct lifetime_req
    {
        expr_req default_construct = expr_req::no_exception;
        expr_req move_construct = expr_req::no_exception;
        expr_req copy_construct = expr_req::no_exception;
        expr_req move_assign = expr_req::no_exception;
        expr_req copy_assign = expr_req::no_exception;
        expr_req destruct = expr_req::no_exception;
        expr_req swap = move_construct;

        template<typename T>
        [[nodiscard]] static constexpr auto for_type() noexcept
        {
            return lifetime_req{
                get_expr_req(std::is_default_constructible_v<T>, std::is_nothrow_default_constructible_v<T>),
                get_expr_req(std::is_move_constructible_v<T>, nothrow_move_constructible<T>),
                get_expr_req(std::is_copy_constructible_v<T>, nothrow_copy_constructible<T>),
                get_expr_req(move_assignable<T>, nothrow_move_assignable<T>),
                get_expr_req(copy_assignable<T>, nothrow_copy_assignable<T>),
                get_expr_req(std::is_destructible_v<T>, std::is_nothrow_destructible_v<T>),
                get_expr_req(std::swappable<T>, nothrow_swappable<T>) //
            };
        }

        [[nodiscard]] static constexpr lifetime_req trivial() noexcept { return {}; };

        [[nodiscard]] static constexpr lifetime_req normal() noexcept
        {
            return {.copy_construct = expr_req::well_formed, .copy_assign = expr_req::well_formed};
        }

        [[nodiscard]] static constexpr lifetime_req unique() noexcept
        {
            return {.copy_construct = expr_req::ill_formed, .copy_assign = expr_req::ill_formed};
        }

        [[nodiscard]] static constexpr lifetime_req ill_formed() noexcept
        {
            return {
                expr_req::ill_formed,
                expr_req::ill_formed,
                expr_req::ill_formed,
                expr_req::ill_formed,
                expr_req::ill_formed,
                expr_req::ill_formed,
                expr_req::ill_formed,
            };
        }

    private:
        using ordering = std::partial_ordering;

        static constexpr auto cmp_impl(ordering& pre, const ordering next) noexcept
        {
            if(is_eq(pre))
            {
                pre = next;
                return true;
            }

            if(pre != next && is_neq(next))
            {
                pre = ordering::unordered;
                return false;
            }

            return true;
        };

        [[nodiscard]] friend constexpr ordering
            operator<=>(const lifetime_req left, const lifetime_req right) noexcept
        {
            ordering cmp = left.default_construct <=> right.default_construct;
            cmp_impl(cmp, left.move_construct <=> right.move_construct) || //
                cmp_impl(cmp, left.copy_construct <=> right.copy_construct) || //
                cmp_impl(cmp, left.move_assign <=> right.move_assign) || //
                cmp_impl(cmp, left.copy_assign <=> right.copy_assign) || //
                cmp_impl(cmp, left.destruct <=> right.destruct) || //
                cmp_impl(cmp, left.swap <=> right.swap);

            return cmp;
        }

        [[nodiscard]] friend constexpr auto
            operator==(const lifetime_req left, const lifetime_req right) noexcept

        {
            return left.default_construct == right.default_construct &&
                left.move_construct == right.move_construct &&
                left.copy_construct == right.copy_construct &&
                left.move_assign == right.move_assign && //
                left.copy_assign == right.copy_assign && //
                left.destruct == right.destruct && //
                left.swap == right.swap;
        }
    };

    [[nodiscard]] constexpr lifetime_req
        minimize(const lifetime_req left, const lifetime_req right) noexcept
    {
        return {
            std::min(left.default_construct, right.default_construct),
            std::min(left.move_construct, right.move_construct),
            std::min(left.copy_construct, right.copy_construct),
            std::min(left.move_assign, right.move_assign),
            std::min(left.copy_assign, right.copy_assign),
            std::min(left.destruct, right.destruct),
            std::min(left.swap, right.swap) //
        };
    }

    [[nodiscard]] constexpr lifetime_req
        maximize(const lifetime_req left, const lifetime_req right) noexcept
    {
        return {
            std::max(left.default_construct, right.default_construct),
            std::max(left.move_construct, right.move_construct),
            std::max(left.copy_construct, right.copy_construct),
            std::max(left.move_assign, right.move_assign),
            std::max(left.copy_assign, right.copy_assign),
            std::max(left.destruct, right.destruct),
            std::max(left.swap, right.swap) //
        };
    }

    template<lifetime_req Req>
    struct fake_type // NOLINTBEGIN(*-use-equals-default, *-noexcept-*)
    {
        constexpr fake_type() noexcept(is_noexcept(Req.default_construct))
            requires(is_well_formed(Req.default_construct))
        {
        }

        constexpr fake_type(fake_type&& /*unused*/) noexcept(is_noexcept(Req.move_construct))
            requires(is_well_formed(Req.move_construct))
        {
        }

        constexpr fake_type(const fake_type& /*unused*/) noexcept(is_noexcept(Req.copy_construct))
            requires(is_well_formed(Req.copy_construct))
        {
        }

        constexpr fake_type& operator=(fake_type&& /*unused*/) noexcept(is_noexcept(Req.move_assign)
        )
            requires(is_well_formed(Req.move_assign))
        {
            return *this;
        }

        constexpr fake_type&
            operator=(const fake_type& /*unused*/) noexcept(is_noexcept(Req.copy_assign))
            requires(is_well_formed(Req.copy_assign))
        {
            return *this;
        }

        constexpr ~fake_type() noexcept(is_noexcept(Req.destruct))
            requires(is_well_formed(Req.destruct))
        {
        }
    }; // NOLINTEND(*-use-equals-default,*-noexcept-*)
}