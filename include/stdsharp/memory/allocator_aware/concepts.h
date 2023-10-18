#pragma once

#include "allocation_traits.h"

namespace stdsharp::allocator_aware
{
    template<typename T, typename Allocator>
    concept allocation_constructible = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation& allocation,
        decltype(traits)::allocator_type& alloc
    ) { decltype(traits)::template construct<T>(allocation, alloc); };

    template<typename T, typename Allocator>
    concept allocation_destructible = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation& allocation,
        decltype(traits)::allocator_type& alloc
    ) { decltype(traits)::template destroy<T>(allocation, alloc); };

    template<typename T, typename Allocator>
    concept allocation_copy_constructible = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation_cref allocation,
        decltype(traits)::allocator_type alloc
    ) { decltype(traits)::template on_construct<T>(allocation, alloc); };

    template<typename T, typename Allocator>
    concept allocation_move_constructible = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation& allocation,
        decltype(traits)::allocator_type alloc
    ) { decltype(traits)::template on_construct<T>(allocation, alloc); };

    template<typename T, typename Allocator>
    concept allocation_copy_assignable = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation_cref src_allocation,
        decltype(traits)::allocator_cref src_alloc,
        decltype(traits)::allocation& dst_allocation,
        decltype(traits)::allocator_type& dst_alloc
    ) {
        decltype(traits):: //
            template on_assign<T>(src_allocation, src_alloc, dst_allocation, dst_alloc);
    };

    template<typename T, typename Allocator>
    concept allocation_move_assignable = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation& src_allocation,
        decltype(traits)::allocator_type& src_alloc,
        decltype(traits)::allocation& dst_allocation,
        decltype(traits)::allocator_type& dst_alloc
    ) {
        decltype(traits):: //
            template on_assign<T>(src_allocation, src_alloc, dst_allocation, dst_alloc);
    };

    template<typename T, typename Allocator>
    concept allocation_swappable = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation& lhs_allocation,
        decltype(traits)::allocator_type& lhs_alloc,
        decltype(traits)::allocation& rhs_allocation,
        decltype(traits)::allocator_type& rhs_alloc
    ) {
        decltype(traits)::template on_swap<T>(lhs_allocation, lhs_alloc, rhs_allocation, rhs_alloc);
    };

    template<typename T, typename Allocator>
    concept allocation_nothrow_constructible = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation& allocation,
        decltype(traits)::allocator_type& alloc
    ) { requires noexcept(decltype(traits)::template construct<T>(allocation, alloc)); };

    template<typename T, typename Allocator>
    concept allocation_nothrow_destructible = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation& allocation,
        decltype(traits)::allocator_type& alloc
    ) { requires noexcept(decltype(traits)::template destroy<T>(allocation, alloc)); };

    template<typename T, typename Allocator>
    concept allocation_nothrow_copy_constructible = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation_cref allocation,
        decltype(traits)::allocator_type alloc
    ) { requires noexcept(decltype(traits)::template on_construct<T>(allocation, alloc)); };

    template<typename T, typename Allocator>
    concept allocation_nothrow_move_constructible = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation& allocation,
        decltype(traits)::allocator_type alloc
    ) { requires noexcept(decltype(traits)::template on_construct<T>(allocation, alloc)); };

    template<typename T, typename Allocator>
    concept allocation_nothrow_copy_assignable = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation_cref src_allocation,
        decltype(traits)::allocator_cref src_alloc,
        decltype(traits)::allocation& dst_allocation,
        decltype(traits)::allocator_type& dst_alloc
    ) {
        requires noexcept(
            decltype(traits):: //
            template on_assign<T>(src_allocation, src_alloc, dst_allocation, dst_alloc)
        );
    };

    template<typename T, typename Allocator>
    concept allocation_nothrow_move_assignable = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation& src_allocation,
        decltype(traits)::allocator_type& src_alloc,
        decltype(traits)::allocation& dst_allocation,
        decltype(traits)::allocator_type& dst_alloc
    ) {
        requires noexcept(
            decltype(traits):: //
            template on_assign<T>(src_allocation, src_alloc, dst_allocation, dst_alloc)
        );
    };

    template<typename T, typename Allocator>
    concept allocation_nothrow_swappable = requires(
        allocation_traits<Allocator> traits,
        decltype(traits)::allocation& lhs_allocation,
        decltype(traits)::allocator_type& lhs_alloc,
        decltype(traits)::allocation& rhs_allocation,
        decltype(traits)::allocator_type& rhs_alloc
    ) {
        requires noexcept( //
            decltype(traits):: //
            template on_swap<T>(lhs_allocation, lhs_alloc, rhs_allocation, rhs_alloc)
        );
    };
}