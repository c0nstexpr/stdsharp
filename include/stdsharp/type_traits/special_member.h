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
        expr_req destroy = expr_req::no_exception;

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
            return is_gt(left) || (is_eq(left) && is_lteq(right)) || is_lt(right) ?
                left :
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

            return cmp == partial_ordering::unordered ?
                cmp :
                compatible(cmp, left.destroy <=> right.destroy);
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
                ::std::min(left.copy_assign, right.copy_assign) //
            };
        }

        friend constexpr special_mem_req
            max(const special_mem_req left, const special_mem_req right) noexcept
        {
            return {
                ::std::max(left.move_construct, right.move_construct),
                ::std::max(left.copy_construct, right.copy_construct),
                ::std::max(left.move_assign, right.move_assign),
                ::std::max(left.copy_assign, right.copy_assign) //
            };
        }
    };

    template<typename T>
    inline constexpr special_mem_req special_mem_req::for_type{
        ::std::move_constructible<T> ?
            (nothrow_move_constructible<T> ? expr_req::no_exception : expr_req::well_formed) :
            expr_req::ill_formed,
        ::std::copy_constructible<T> ?
            (nothrow_copy_constructible<T> ? expr_req::no_exception : expr_req::well_formed) :
            expr_req::ill_formed,
        move_assignable<T> ?
            (nothrow_move_assignable<T> ? expr_req::no_exception : expr_req::well_formed) :
            expr_req::ill_formed,
        copy_assignable<T> ?
            (nothrow_copy_assignable<T> ? expr_req::no_exception : expr_req::well_formed) :
            expr_req::ill_formed //
    };

    inline constexpr special_mem_req special_mem_req::trivial{
        expr_req::no_exception,
        expr_req::no_exception,
        expr_req::no_exception,
        expr_req::no_exception //
    };

    inline constexpr special_mem_req special_mem_req::normal{
        expr_req::no_exception,
        expr_req::well_formed,
        expr_req::no_exception,
        expr_req::well_formed,
    };

    inline constexpr special_mem_req special_mem_req::normal_movable{
        expr_req::no_exception,
        expr_req::ill_formed,
        expr_req::no_exception,
        expr_req::ill_formed,
    };
}