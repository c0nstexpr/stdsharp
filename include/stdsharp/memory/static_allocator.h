#pragma once

#include "static_memory_resource.h"
#include "../cmath/cmath.h"
#include "pointer_traits.h"

namespace stdsharp
{
    template<typename T, ::std::size_t ResourceSize>
    class static_allocator
    {
    public:
        using value_type = T;
        using propagate_on_container_move_assignment = ::std::true_type;
        using propagate_on_container_copy_assignment = ::std::true_type;
        using propagate_on_container_swap = ::std::true_type;

        static constexpr auto size = ResourceSize;

        using resource_type = static_memory_resource<size>;

        constexpr explicit static_allocator(resource_type& src) noexcept: src_(src) {}

        template<typename U>
        struct rebind
        {
            using other = static_allocator<U, ResourceSize>;
        };

        template<typename U>
        constexpr static_allocator(const typename rebind<U>::other other) noexcept:
            static_allocator(other.resource())
        {
        }

        [[nodiscard]] constexpr T* allocate(const ::std::size_t required_size)
        {
            return pointer_cast<T>(resource().allocate(to_generic_size(required_size)));
        }

        [[nodiscard]] constexpr T* try_allocate(const ::std::size_t required_size)
        {
            return pointer_cast<T>(resource().try_allocate(to_generic_size(required_size)));
        }

        constexpr void deallocate(T* const ptr, const ::std::size_t required_size) noexcept
        {
            if(ptr == nullptr) return;
            resource().deallocate(
                pointer_cast<generic_storage>(ptr),
                to_generic_size(required_size)
            );
        }

        [[nodiscard]] constexpr resource_type& resource() const noexcept { return src_.get(); }

        [[nodiscard]] constexpr bool operator==(const static_allocator other) const noexcept
        {
            return resource() == other.resource();
        }

        [[nodiscard]] constexpr bool contains(const T* const ptr) const noexcept
        {
            return resource().contains(pointer_cast<generic_storage>(ptr));
        }

    private:
        ::std::reference_wrapper<resource_type> src_;

        static constexpr auto to_generic_size(const ::std::size_t size_v)
        {
            return ceil_reminder(size_v * sizeof(T), sizeof(generic_storage));
        }
    };

    template<::std::size_t Size>
    static_allocator(static_memory_resource<Size>&) -> static_allocator<generic_storage, Size>;

    template<typename T, ::std::size_t Size>
    using static_allocator_for =
        static_allocator<T, ceil_reminder(Size * sizeof(T), sizeof(generic_storage))>;
}