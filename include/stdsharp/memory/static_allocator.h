#pragma once

#include "static_memory_resource.h"

namespace stdsharp
{
    template<::std::size_t Size>
    class static_allocator
    {
    public:
        constexpr const auto& resource() const noexcept { return resource_; }

    private:
        static_memory_resource<Size>& resource_{};
    };
}