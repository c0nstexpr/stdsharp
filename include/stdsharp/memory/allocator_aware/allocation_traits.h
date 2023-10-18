#pragma once

#include "allocation.h"

namespace stdsharp::allocator_aware
{
    template<allocator_req Alloc>
    struct allocation_traits
    {
        using allocator_type = Alloc;
        using allocator_traits = allocator_traits<allocator_type>;
        using value_type = allocator_traits::value_type;
        using size_type = allocator_traits::size_type;
        using cvp = allocator_traits::const_void_pointer;
        using pointer = allocator_traits::pointer;
        using const_pointer = allocator_traits::const_pointer;

        using allocation = allocation<allocator_type>;
        class callocation;

        using allocation_cref = const callocation&;
        using allocator_cref = const allocator_type&;

        static constexpr allocation
            allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr)
        {
            return {allocator_traits::allocate(alloc, size, hint), size};
        }

        static constexpr allocation try_allocate(
            allocator_type& alloc,
            const size_type size,
            const cvp hint = nullptr
        ) noexcept
        {
            return {allocator_traits::try_allocate(alloc, size, hint), size};
        }

        static constexpr void deallocate(allocation& allocation, allocator_type& alloc) noexcept
        {
            allocator_traits::deallocate(alloc, allocation.data(), allocation.size());
            allocation = {};
        }

        template<typename T, typename... Args>
            requires(allocator_traits::template constructible_from<T, Args...>)
        static constexpr T&
            construct(allocation& allocation, allocator_type& alloc, Args&&... args) //
            noexcept(allocator_traits::template nothrow_constructible_from<T, Args...>)
        {
            Expects(allocation.size() >= sizeof(T));

            allocator_traits::construct(alloc, allocation.template data<T>(), cpp_forward(args)...);

            return allocation.template get<T>();
        }

        template<typename T>
            requires(allocator_traits::template destructible<T>)
        static constexpr void destroy(allocation& allocation, allocator_type& alloc) //
            noexcept(allocator_traits::template nothrow_destructible<T>)
        {
            if(allocation.empty()) return;
            allocator_traits::destroy(alloc, allocation.template data<T>());
        }

        template<typename T>
            requires(allocator_traits::template cp_constructible<T>)
        [[nodiscard]] static constexpr allocation
            on_construct(allocation_cref src_allocation, allocator_type& alloc) //
        {
            auto allocation = allocate(alloc, sizeof(T));
            construct<T>(allocation, alloc, src_allocation.template cget<T>());
            return allocation;
        }

        template<std::move_constructible>
        [[nodiscard]] static constexpr allocation
            on_construct(allocation& allocation, allocator_type& /*unused*/) noexcept
        {
            return std::exchange(allocation, {});
        }

    private:
        static constexpr auto always_equal_v = allocator_traits::always_equal_v;

        static constexpr void validate_allocations_on_assign(
            allocation_cref src_allocation, // NOLINT(*-swappable-parameters)
            allocation_cref dst_allocation
        )
        {
            Expects(!src_allocation.empty());
            Expects(!dst_allocation.empty());
        }

    public:
        template<copy_assignable T>
        static constexpr void on_assign(
            allocation_cref src_allocation,
            allocator_cref src_alloc,
            allocation& dst_allocation,
            allocator_type& dst_alloc
        ) noexcept(nothrow_copy_assignable<T>)
            requires allocator_traits::propagate_on_copy_v && always_equal_v
        {
            validate_allocations_on_assign(src_allocation, dst_allocation);
            dst_alloc = src_alloc;
            dst_allocation.template get<T>() = src_allocation.template cget<T>();
        }

        template<copy_assignable T>
        static constexpr void on_assign(
            allocation_cref src_allocation,
            allocator_cref src_alloc,
            allocation& dst_allocation,
            allocator_type& dst_alloc
        )
            requires requires {
                requires allocator_traits::propagate_on_copy_v;
                destroy<T>(dst_allocation, dst_alloc);
                on_construct<T>(src_allocation, dst_alloc);
            }
        {
            validate_allocations_on_assign(src_allocation, dst_allocation);
            if(dst_alloc == src_alloc)
            {
                dst_alloc = src_alloc;
                dst_allocation.template get<T>() = src_allocation.template cget<T>();
            }
            else
            {
                destroy<T>(dst_allocation, dst_alloc);
                deallocate(dst_allocation, dst_alloc);
                dst_alloc = src_alloc;
                dst_allocation = on_construct<T>(src_allocation, dst_alloc);
            }
        }

        template<copy_assignable T>
        static constexpr void on_assign(
            allocation_cref src_allocation,
            allocator_cref /*unused*/,
            allocation& dst_allocation,
            allocator_cref /*unused*/
        ) noexcept(nothrow_copy_assignable<T>)
        {
            validate_allocations_on_assign(src_allocation, dst_allocation);
            dst_allocation.template get<T>() = src_allocation.template cget<T>();
        }

    private:
        template<typename T>
        static constexpr void mov_allocation(
            allocation& src_allocation,
            allocation& dst_allocation,
            allocator_type& dst_alloc
        ) noexcept(noexcept(destroy<T>(dst_allocation, dst_alloc)))
            requires requires { destroy<T>(dst_allocation, dst_alloc); }
        {
            destroy<T>(dst_allocation, dst_alloc);
            deallocate(dst_allocation, dst_alloc);
            dst_allocation = std::exchange(src_allocation, {});
        }

    public:
        template<move_assignable T>
        static constexpr void on_assign(
            allocation& src_allocation,
            allocator_type& src_alloc,
            allocation& dst_allocation,
            allocator_type& dst_alloc
        ) noexcept(noexcept(mov_allocation<T>(src_allocation, dst_allocation, dst_alloc)))
            requires requires {
                requires allocator_traits::propagate_on_move_v;
                mov_allocation<T>(src_allocation, dst_allocation, dst_alloc);
            }
        {
            validate_allocations_on_assign(src_allocation, dst_allocation);
            mov_allocation<T>(src_allocation, dst_allocation, dst_alloc);
            dst_alloc = cpp_move(src_alloc);
        }

        template<move_assignable T>
        static constexpr void on_assign(
            allocation& src_allocation,
            allocator_cref /*unused*/,
            allocation& dst_allocation,
            allocator_type& dst_alloc
        ) noexcept(noexcept(mov_allocation<T>(src_allocation, dst_allocation, dst_alloc)))
            requires requires {
                requires !allocator_traits::propagate_on_move_v;
                requires allocator_traits::always_equal_v;
                mov_allocation<T>(src_allocation, dst_allocation, dst_alloc);
            }
        {
            validate_allocations_on_assign(src_allocation, dst_allocation);
            mov_allocation<T>(src_allocation, dst_allocation, dst_alloc);
        }

        template<move_assignable T>
        static constexpr void on_assign(
            allocation& src_allocation,
            allocator_cref /*unused*/,
            allocation& dst_allocation,
            allocator_cref /*unused*/
        ) noexcept(nothrow_move_assignable<T>)
        {
            validate_allocations_on_assign(src_allocation, dst_allocation);
            dst_allocation.template get<T>() = cpp_move(src_allocation.template get<T>());
        }

        template<std::swappable T>
        static constexpr void on_swap(
            allocation& lhs_allocation,
            allocator_type& lhs_alloc,
            allocation& rhs_allocation,
            allocator_type& rhs_alloc
        ) noexcept
        {
            if constexpr(allocator_traits::propagate_on_swap_v) std::swap(lhs_alloc, rhs_alloc);
            else if constexpr(!always_equal_v) Expects(lhs_alloc == rhs_alloc);
            std::swap(lhs_allocation, rhs_allocation);
        }
    };
}