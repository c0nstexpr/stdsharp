#pragma once

#include "allocator_traits.h"

namespace stdsharp
{
    template<allocator_req Alloc>
    class allocation
    {
        Alloc alloc_;

        using traits = allocator_traits<Alloc>;
        using pointer = typename traits::pointer;
        using size_type = typename traits::size_type;

        pointer ptr_;
        size_type size_;

    public:
        allocation() = default;
    };
}