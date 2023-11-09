#pragma once

#include "../algorithm/algorithm.h"
#include "stdsharp/type_traits/core_traits.h"

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

        [[nodiscard]] static constexpr auto trivial() noexcept { return lifetime_req{}; };

        // TODO: replace with designated initializer when msvc fix
        [[nodiscard]] static constexpr auto normal() noexcept
        {
            lifetime_req res{};
            res.copy_construct = expr_req::well_formed;
            res.copy_assign = expr_req::well_formed;
            return res;
        }

        [[nodiscard]] static constexpr auto unique() noexcept
        {
            lifetime_req res{};
            res.copy_construct = expr_req::ill_formed;
            res.copy_assign = expr_req::ill_formed;
            return res;
        }

        [[nodiscard]] static constexpr auto ill_formed() noexcept
        {
            return lifetime_req{
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
        [[nodiscard]] constexpr auto to_rng() const noexcept
        {
            return std::array{
                default_construct,
                move_construct,
                copy_construct,
                move_assign,
                copy_assign,
                destruct,
                swap
            };
        }

        [[nodiscard]] friend constexpr std::partial_ordering
            operator<=>(const lifetime_req left, const lifetime_req right) noexcept
        {
            return strict_compare(left.to_rng(), right.to_rng());
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
        min(const lifetime_req left, const lifetime_req right) noexcept
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
        max(const lifetime_req left, const lifetime_req right) noexcept
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
    struct fake_type_for : empty_t // NOLINTBEGIN(*-use-equals-default,*-noexcept-*)
    {
        fake_type_for() noexcept(is_noexcept(Req.default_construct))
            requires(is_well_formed(Req.default_construct))
        = default;

        fake_type_for(fake_type_for&&) noexcept(is_noexcept(Req.move_construct))
            requires(is_well_formed(Req.move_construct))
        = default;

        fake_type_for(const fake_type_for&) noexcept(is_noexcept(Req.copy_construct))
            requires(is_well_formed(Req.copy_construct))
        = default;

        fake_type_for& operator=(fake_type_for&&) noexcept(is_noexcept(Req.move_assign))
            requires(is_well_formed(Req.move_assign))
        = default;

        fake_type_for& operator=(const fake_type_for&) noexcept(is_noexcept(Req.copy_assign))
            requires(is_well_formed(Req.copy_assign))
        = default;

        ~fake_type_for() noexcept(is_noexcept(Req.destruct))
            requires(is_well_formed(Req.destruct))
        = default;
    }; // NOLINTEND(*-use-equals-default,*-noexcept-*)
}