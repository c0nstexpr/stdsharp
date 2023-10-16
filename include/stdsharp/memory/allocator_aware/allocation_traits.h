#pragma once

#include "../allocator_traits.h"

namespace stdsharp::details
{
    template<typename>
    struct allocation_traits_value_type
    {
    };

    template<typename T>
        requires requires { typename T::value_type; }
    struct allocation_traits_value_type<T>
    {
        using value_type = T::value_type;
    };
}

namespace stdsharp::allocator_aware
{
    template<nothrow_default_initializable Allocation>
        requires requires(
            Allocation allocation,
            const Allocation callocation,
            Allocation::allocator_type alloc,
            allocator_traits<decltype(alloc)>::size_type n,
            const Allocation::const_void_pointer hint
        ) {
            requires nothrow_copyable<Allocation>;
            {
                callocation.size()
            } noexcept -> std::convertible_to<decltype(n)>;

            {
                callocation.empty()
            } noexcept -> boolean_testable;

            alloc.deallocate(alloc);
        }
    struct allocation_traits
    {
        using allocation_type = Allocation;
        using callocation = const allocation_type;
        using allocator_type = allocation_type::allocator_type;

        using allocator_traits = allocator_traits<allocator_type>;
        using cvp = allocator_traits::const_void_pointer;
        using size_type = allocator_traits::size_type;

        [[nodiscard]] static constexpr size_type size(callocation& allocation) noexcept
        {
            return allocation.size();
        }

        [[nodiscard]] static constexpr bool empty(callocation& allocation) noexcept
        {
            return allocation.empty();
        }

        template<typename... Args>
        static constexpr void
            allocate(allocation_type& allocation, allocator_type& alloc, Args&&... args)
            requires requires { allocation.allocate(alloc, cpp_forward(args)...); }
        {
            return allocation.allocate(alloc, cpp_forward(args)...);
        }

        static constexpr void deallocate(allocation_type& allocation, allocator_type& alloc) //
            noexcept(noexcept(allocation.deallocate(alloc)))
        {
            return allocation.deallocate(alloc);
        }

        static constexpr void
            construct(allocation_type& allocation, allocator_type& alloc, auto&&... args)
            requires requires { allocation.construct(alloc, cpp_forward(args)...); }
        {
            return allocation.construct(alloc, cpp_forward(args)...);
        }

        static constexpr void destroy(allocation_type& allocation, allocator_type& alloc) //
            noexcept(noexcept(allocation.destroy(alloc)))
            requires requires { allocation.destroy(alloc); }
        {
            return allocation.destroy(alloc);
        }

        [[nodiscard]] static constexpr allocation_type
            cp_construct(callocation& allocation, allocator_type& alloc) //
            noexcept(noexcept(allocation.cp_construct(alloc)))
            requires requires {
                {
                    allocation.cp_construct(alloc)
                } -> std::same_as<allocation_traits::allocation_type>;
            }
        {
            return allocation.cp_construct(alloc);
        }

        [[nodiscard]] static constexpr allocation_type
            mov_construct(allocation_type& allocation, allocator_type& alloc) //
            noexcept(noexcept(allocation.mov_construct(alloc)))
            requires requires {
                {
                    allocation.mov_construct(alloc)
                } -> std::same_as<allocation_type>;
            }
        {
            return allocation.mov_construct(alloc);
        }

        static constexpr void cp_assign(
            callocation& allocation,
            const allocator_type& alloc,
            allocation_type& other,
            allocator_type& other_alloc
        ) noexcept(noexcept(allocation.cp_assign(alloc, other_alloc, other)))
            requires requires { allocation.cp_assign(alloc, other_alloc, other); }
        {
            return allocation.cp_assign(alloc, other_alloc, other);
        }

        static constexpr void mov_assign(
            allocation_type& allocation,
            allocator_type& alloc,
            allocation_type& other,
            allocator_type& other_alloc
        ) noexcept(noexcept(allocation.mov_assign(alloc, other_alloc, other)))
            requires requires { allocation.mov_assign(alloc, other_alloc, other); }
        {
            return allocation.mov_assign(alloc, other_alloc, other);
        }


    private:
        static constexpr bool
            vaildate_alloc(const allocator_type& left, const allocator_type& right) noexcept
        {
            if constexpr(allocator_traits::always_equal_v) return true;
            else return left == right;
        }

    public:
        static constexpr void swap(
            allocation_type& src_allocation,
            allocator_type& src_alloc,
            allocation_type& dst_allocation,
            allocator_type& dst_alloc
        ) noexcept
        {
            Expects((vaildate_alloc(src_alloc, dst_alloc)));
            std::swap(src_allocation, dst_allocation);
            if constexpr(allocator_traits::propagate_on_swap_v)
                std::ranges::swap(dst_alloc, src_alloc);
        }
    };
}