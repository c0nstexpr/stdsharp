#pragma once

#include "allocation.h"

namespace stdsharp::allocator_aware
{
    template<allocator_req Allocator>
    struct allocation_traits
    {
        using allocator_type = Allocator;
        using allocator_traits = allocator_traits<allocator_type>;
        using value_type = allocator_traits::value_type;
        using size_type = allocator_traits::size_type;
        using difference_type = allocator_traits::difference_type;
        using cvp = allocator_traits::const_void_pointer;
        using pointer = allocator_traits::pointer;
        using const_pointer = allocator_traits::const_pointer;
        using void_pointer = allocator_traits::void_pointer;
        using const_void_pointer = allocator_traits::const_void_pointer;

        using allocation_type = allocation<allocator_type>;
        using callocation = callocation<allocation_type>;

        using allocator_cref = const allocator_type&;

        static constexpr allocation_type
            allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr)
        {
            return {allocator_traits::allocate(alloc, size, hint), size};
        }

        static constexpr allocation_type
            try_allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr) //
            noexcept
        {
            return {allocator_traits::try_allocate(alloc, size, hint), size};
        }

        template<typename T>
        static constexpr void deallocate(const target_allocations<allocator_type, T> dst) noexcept
        {
            for(allocation_type& allocation : dst.allocations)
            {
                allocator_traits::deallocate(dst.allocator, allocation.data(), allocation.size());
                allocation = {};
            }
        }
    };
}