#pragma once

#include <algorithm>
#include <compare>

#include "../concepts/concepts.h"
#include "core_traits.h"

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
        static const special_mem_req normal_movable;

    private:
        using partial_ordering = ::std::partial_ordering;
        using strong_ordering = ::std::strong_ordering;

        static constexpr auto
            compatible(const partial_ordering left, const partial_ordering right) noexcept
        {
            if(left == right) return left;
            if(is_eq(left)) return right;
            if(is_eq(right)) return left;

            return is_gt(left) ?
                is_gt(right) ? partial_ordering::greater : partial_ordering::unordered :
                is_lt(right) ? partial_ordering::less :
                               partial_ordering::unordered;
        }

        friend constexpr partial_ordering
            operator<=>(const special_mem_req left, const special_mem_req right) noexcept
        {
            auto cmp = compatible(
                left.move_construct <=> right.move_construct,
                left.copy_construct <=> right.copy_construct
            );

            if(cmp == partial_ordering::unordered) return cmp;

            cmp = compatible(cmp, left.move_assign <=> right.move_assign);

            if(cmp == partial_ordering::unordered) return cmp;

            cmp = compatible(cmp, left.copy_assign <=> right.copy_assign);

            if(cmp == partial_ordering::unordered) return cmp;

            cmp = compatible(cmp, left.destruct <=> right.destruct);

            return cmp == partial_ordering::unordered ? //
                cmp :
                compatible(cmp, left.swap <=> right.swap);
        }

        friend constexpr auto
            operator==(const special_mem_req left, const special_mem_req right) noexcept
        {
            return (left <=> right) == partial_ordering::equivalent;
        }

        friend constexpr special_mem_req
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

        friend constexpr special_mem_req
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
    };

    template<typename T>
    inline constexpr special_mem_req special_mem_req::for_type{
        ::std::move_constructible<T> ?
            nothrow_move_constructible<T> ? expr_req::no_exception : expr_req::well_formed :
            expr_req::ill_formed,
        ::std::copy_constructible<T> ?
            nothrow_copy_constructible<T> ? expr_req::no_exception : expr_req::well_formed :
            expr_req::ill_formed,
        move_assignable<T> ?
            nothrow_move_assignable<T> ? expr_req::no_exception : expr_req::well_formed :
            expr_req::ill_formed,
        copy_assignable<T> ?
            nothrow_copy_assignable<T> ? expr_req::no_exception : expr_req::well_formed :
            expr_req::ill_formed,
        ::std::is_destructible_v<T> ?
            ::std::is_nothrow_destructible_v<T> ? expr_req::no_exception : expr_req::well_formed :
            expr_req::ill_formed,
        ::std::swappable<T> ?
            nothrow_swappable<T> ? expr_req::no_exception : expr_req::well_formed :
            expr_req::ill_formed //
    };

    inline constexpr special_mem_req special_mem_req::trivial{};

    inline constexpr special_mem_req special_mem_req::normal{
        .copy_construct = expr_req::well_formed,
        .copy_assign = expr_req::well_formed,
    };

    inline constexpr special_mem_req special_mem_req::normal_movable{
        .copy_construct = expr_req::ill_formed,
        .copy_assign = expr_req::ill_formed,
    };
}