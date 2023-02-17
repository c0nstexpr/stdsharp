#pragma once

#include "static_memory_resource.h"

namespace stdsharp
{
    template<typename T, ::std::size_t ByteSize>
    class static_allocator
    {
    public:
        using value_type = T;
        using propagate_on_container_move_assignment = ::std::true_type;
        using propagate_on_container_copy_assignment = ::std::true_type;
        using propagate_on_container_swap = ::std::true_type;

        static constexpr auto size = ByteSize;

        using resource_type = static_memory_resource<size>;

        constexpr explicit static_allocator(resource_type& src) noexcept: src_(src) {}

        template<typename U>
        struct rebind
        {
            using other = static_allocator<U, ByteSize>;
        };

        template<typename U>
        constexpr static_allocator(const typename rebind<U>::other other) noexcept:
            static_allocator(other.resource())
        {
        }

        [[nodiscard]] constexpr T* allocate(const ::std::size_t required_size)
        {
            return point_as<T>(resource().allocate(required_size * sizeof(T)));
        }

        [[nodiscard]] constexpr T* try_allocate(const ::std::size_t required_size)
        {
            return resource().try_allocate(required_size * sizeof(T));
        }

        constexpr void deallocate(T* const ptr, const ::std::size_t required_size) noexcept
        {
            if(ptr == nullptr) return;
            resource().deallocate(ptr, required_size * sizeof(T));
        }

        [[nodiscard]] constexpr resource_type& resource() const noexcept { return src_.get(); }

        [[nodiscard]] constexpr bool operator==(const static_allocator other) const noexcept
        {
            return resource() == other.resource();
        }

    private:
        ::std::reference_wrapper<resource_type> src_;
    };

    template<typename T, ::std::size_t Size>
    using static_allocator_for = static_allocator<T, Size * sizeof(T)>;
}