#pragma once

#include "../algorithm/algorithm.h"

namespace stdsharp
{
    struct special_mem_req
    {
        expr_req move_construct = expr_req::no_exception;
        expr_req copy_construct = expr_req::no_exception;
        expr_req move_assign = expr_req::no_exception;
        expr_req copy_assign = expr_req::no_exception;
        expr_req destruct = expr_req::no_exception;
        expr_req swap = move_construct;

        template<typename T>
        static const special_mem_req for_type;
        static const special_mem_req trivial;
        static const special_mem_req normal;
        static const special_mem_req unique;
        static const special_mem_req ill_formed;

    private:
        [[nodiscard]] constexpr auto to_rng() const noexcept
        {
            return ::std::array{
                move_construct,
                copy_construct,
                move_assign,
                copy_assign,
                destruct,
                swap //
            };
        }

        [[nodiscard]] friend constexpr ::std::partial_ordering
            operator<=>(const special_mem_req left, const special_mem_req right) noexcept
        {
            return strict_compare(left.to_rng(), right.to_rng());
        }

        [[nodiscard]] friend constexpr auto
            operator==(const special_mem_req left, const special_mem_req right) noexcept

        {
            return left.move_construct == right.move_construct &&
                left.copy_construct == right.copy_construct &&
                left.move_assign == right.move_assign && //
                left.copy_assign == right.copy_assign && //
                left.destruct == right.destruct && //
                left.swap == right.swap;
        }
    };

    [[nodiscard]] constexpr special_mem_req
        min(const special_mem_req left, const special_mem_req right) noexcept
    {
        return {
            ::std::min(left.move_construct, right.move_construct),
            ::std::min(left.copy_construct, right.copy_construct),
            ::std::min(left.move_assign, right.move_assign),
            ::std::min(left.copy_assign, right.copy_assign),
            ::std::min(left.destruct, right.destruct),
            ::std::min(left.swap, right.swap) //
        };
    }

    [[nodiscard]] constexpr special_mem_req
        max(const special_mem_req left, const special_mem_req right) noexcept
    {
        return {
            ::std::max(left.move_construct, right.move_construct),
            ::std::max(left.copy_construct, right.copy_construct),
            ::std::max(left.move_assign, right.move_assign),
            ::std::max(left.copy_assign, right.copy_assign),
            ::std::max(left.destruct, right.destruct),
            ::std::max(left.swap, right.swap) //
        };
    }

    template<typename T>
    inline constexpr special_mem_req special_mem_req::for_type{
        get_expr_req(::std::move_constructible<T>, nothrow_move_constructible<T>),
        get_expr_req(::std::copy_constructible<T>, nothrow_copy_constructible<T>),
        get_expr_req(move_assignable<T>, nothrow_move_assignable<T>),
        get_expr_req(copy_assignable<T>, nothrow_copy_assignable<T>),
        get_expr_req(::std::is_destructible_v<T>, ::std::is_nothrow_destructible_v<T>),
        get_expr_req(::std::swappable<T>, nothrow_swappable<T>) //
    };

    inline constexpr special_mem_req special_mem_req::trivial{};

    inline constexpr special_mem_req special_mem_req::normal{
        .copy_construct = expr_req::well_formed,
        .copy_assign = expr_req::well_formed,
    };

    inline constexpr special_mem_req special_mem_req::unique{
        .copy_construct = expr_req::ill_formed,
        .copy_assign = expr_req::ill_formed,
    };

    inline constexpr special_mem_req special_mem_req::ill_formed{
        expr_req::ill_formed,
        expr_req::ill_formed,
        expr_req::ill_formed,
        expr_req::ill_formed,
        expr_req::ill_formed,
        expr_req::ill_formed,
    };

    template<special_mem_req Req>
    struct fake_type_for : empty_t
    {
        fake_type_for(fake_type_for&&) noexcept(Req.move_construct >= expr_req::no_exception)
            requires(Req.move_construct >= expr_req::well_formed)
        = default;

        fake_type_for(const fake_type_for&) noexcept(Req.copy_construct >= expr_req::no_exception)
            requires(Req.copy_construct >= expr_req::well_formed)
        = default;

        fake_type_for& operator=(fake_type_for&&) //
            noexcept(Req.move_assign >= expr_req::no_exception)
            requires(Req.move_assign >= expr_req::well_formed)
        = default;

        fake_type_for& operator=(const fake_type_for&) //
            noexcept(Req.copy_assign >= expr_req::no_exception)
            requires(Req.copy_assign >= expr_req::well_formed)
        = default;

        ~fake_type_for() noexcept(Req.destruct >= expr_req::no_exception)
            requires(Req.destruct >= expr_req::well_formed)
        = default;
    };
}