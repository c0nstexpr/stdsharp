#pragma once

#include <functional>
#include <memory>

namespace stdsharp
{
    template<typename Alloc>
    class allocator_reference : ::std::reference_wrapper<Alloc>
    {
        using traits = ::std::allocator_traits<Alloc>;

    public:
        using ::std::reference_wrapper<Alloc>::reference_wrapper;

        using value_type = typename traits::value_type;
        using pointer = typename traits::pointer;
        using const_pointer = typename traits::const_pointer;
        using void_pointer = typename traits::void_pointer;
        using const_void_pointer = typename traits::const_void_pointer;
        using difference_type = typename traits::difference_type;
        using size_type = typename traits::size_type;
        using propagate_on_container_copy_assignment =
            typename traits::propagate_on_container_copy_assignment;
        using propagate_on_container_move_assignment =
            typename traits::propagate_on_container_move_assignment;
        using propagate_on_container_swap = typename traits::propagate_on_container_swap;
        using is_always_equal = typename traits::is_always_equal;

        template<typename U>
        struct rebind
        {
            using other = typename traits::template rebind_alloc<U>;
        };

        [[nodiscard]] constexpr auto allocate(const size_type n) const
        {
            return traits::allocate(this->get(), n);
        }

        [[nodiscard]] constexpr auto
            allocate(const size_type n, const const_void_pointer hint) const
        {
            return traits::allocate(this->get(), n, hint);
        }

        [[nodiscard]] constexpr auto allocate_at_least(const size_type n) const
        {
            return traits::allocate_at_least(this->get(), n);
        }

        constexpr void deallocate(const pointer p, const size_type n) const
        {
            traits::deallocate(this->get(), p, n);
        }

        constexpr auto max_size() const noexcept { return traits::max_size(this->get()); }

        constexpr allocator_reference select_on_container_copy_construction() const
        {
            return traits::select_on_container_copy_construction(this->get());
        }
    };
}