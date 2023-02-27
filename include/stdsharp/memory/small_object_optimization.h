#pragma once

#include "allocator_traits.h"

namespace stdsharp
{
    template<allocator_req FirstAlloc, allocator_req SecondAlloc>
        requires ::std::same_as<typename FirstAlloc::value_type, typename SecondAlloc::value_type>
    class composed_allocator
    {
    };
}