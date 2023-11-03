#pragma once

#include "allocation.h"

namespace stdsharp
{
    template<allocator_req Alloc>
    struct allocation_traits
    {
        using allocator_type = Alloc;
        using allocator_traits = allocator_traits<allocator_type>;
        using value_type = allocator_traits::value_type;
        using size_type = allocator_traits::size_type;
        using difference_type = allocator_traits::difference_type;
        using pointer = allocator_traits::pointer;
        using const_pointer = allocator_traits::const_pointer;
        using void_pointer = allocator_traits::void_pointer;
        using cvp = allocator_traits::const_void_pointer;
        using allocation_result = allocation_result<allocator_type>;

        static constexpr auto size = std::ranges::size;

        static constexpr auto empty = std::ranges::empty;

        template<typename T = value_type>
        static constexpr auto data = allocation_data<Alloc, T>;

        template<typename T = value_type>
        static constexpr auto cdata = allocation_cdata<Alloc, T>;

        template<typename T = value_type>
        static constexpr auto get = allocation_get<Alloc, T>;

        template<typename T = value_type>
        static constexpr auto cget = allocation_cget<Alloc, T>;

        template<nothrow_constructible_from<pointer, size_type> Allocation = allocation_result>
        static constexpr allocation<Alloc> auto allocate(
            allocator_type& alloc,
            const size_type size,
            const cvp hint = nullptr //
        )
        {
            return Allocation{allocator_traits::allocate(alloc, size, hint), size};
        }

        template<nothrow_constructible_from<pointer, size_type> Allocation = allocation_result>
        static constexpr allocation<Alloc> auto try_allocate(
            allocator_type& alloc,
            const size_type size,
            const cvp hint = nullptr //
        ) noexcept
        {
            return Allocation{allocator_traits::try_allocate(alloc, size, hint), size};
        }

        template<allocations_view<Alloc> View>
        static constexpr void deallocate(allocator_type& alloc, View&& dst) noexcept
        {
            for(auto& dst_allocation : cpp_forward(dst))
            {
                allocator_traits::deallocate(alloc, data<>(dst_allocation), size(dst_allocation));
                dst_allocation = std::ranges::range_value_t<View>{};
            }
        }

        template<typename T = Alloc::value_type, typename... Args>
            requires(allocator_traits::template constructible_from<T, Args...>)
        static constexpr void construct(
            allocator_type& alloc,
            const allocation<Alloc> auto& allocation,
            Args&&... args
        ) //
            noexcept(allocator_traits::template nothrow_constructible_from<T, Args...>)
        {
            Expects(size(allocation) * sizeof(value_type) >= sizeof(T));
            allocator_traits::construct(alloc, data<T>(allocation), cpp_forward(args)...);
        }

    private:
        template<typename Fn, typename Src, typename Dst>
        static constexpr auto allocation_ctor = std::copy_constructible<Fn> &&
            std::invocable<Fn&,
                           allocator_type&,
                           range_const_reference_t<Src>,
                           range_const_reference_t<Dst>>;

        template<typename Fn, typename Src, typename Dst>
        static constexpr auto nothrow_allocation_ctor = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&,
                              allocator_type&,
                              range_const_reference_t<Src>,
                              range_const_reference_t<Dst>>;

        template<typename Fn, typename Src, typename Dst>
        static constexpr auto allocation_assign = std::copy_constructible<Fn> &&
            std::invocable<Fn&, range_const_reference_t<Src>, range_const_reference_t<Dst>>;

        template<typename Fn, typename Src, typename Dst>
        static constexpr auto nothrow_allocation_assign = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&, range_const_reference_t<Src>, range_const_reference_t<Dst>>;

        template<typename Fn, typename Dst>
        static constexpr auto allocation_dtor = std::copy_constructible<Fn> &&
            std::invocable<Fn&, allocator_type&, range_const_reference_t<Dst>>;

        template<typename Fn, typename Dst>
        static constexpr auto nothrow_allocation_dtor = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&, allocator_type&, range_const_reference_t<Dst>>;

    public:
        template<callocations_view<Alloc> Src, callocations_view<Alloc> Dst, typename Fn>
            requires allocation_ctor<Fn, Src, Dst>
        static constexpr void on_construct(allocator_type& alloc, Src&& src, Dst&& dst, Fn fn) //
            noexcept(nothrow_allocation_ctor<Fn, Src, Dst>)
        {
            for(const auto& [src_allocation, dst_allocation] :
                std::views::zip(cpp_forward(src), cpp_forward(dst)))
                std::invoke(fn, alloc, src_allocation, dst_allocation);
        }

        static constexpr void
            on_construct(allocations_view<Alloc> auto&& src, allocations_view<Alloc> auto&& dst) //
            noexcept
        {
            std::ranges::move(cpp_forward(src), std::ranges::begin(cpp_forward(dst)));
        }

        template<callocations_view<Alloc> Src, callocations_view<Alloc> Dst, typename Fn>
            requires allocation_assign<Fn, Src, Dst>
        static constexpr void on_assign(Src&& src, Dst&& dst, Fn fn) //
            noexcept(nothrow_allocation_assign<Fn, Src, Dst>)
        {
            for(const auto& [src_allocation, dst_allocation] :
                std::views::zip(cpp_forward(src), cpp_forward(dst)))
                std::invoke(fn, src_allocation, dst_allocation);
        }

        template<callocations_view<Alloc> View, typename Fn>
            requires allocation_dtor<Fn, View>
        static constexpr void on_destroy(allocator_type& alloc, View&& dst, Fn fn) //
            noexcept(nothrow_allocation_dtor<Fn, View>)
        {
            for(const auto& allocation : cpp_forward(dst)) std::invoke(fn, alloc, allocation);
        }

        static constexpr void
            on_swap(allocations_view<Alloc> auto&& lhs, allocations_view<Alloc> auto&& rhs) noexcept
        {
            std::ranges::swap_ranges(lhs, rhs);
        }
    };
}