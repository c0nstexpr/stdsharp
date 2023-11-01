#pragma once

#include "allocation.h"

namespace stdsharp::allocator_aware::details
{
    template<typename Allocator, typename Allocation>
    struct allocation_rng_traits
    {
        using allocator_traits = allocator_traits<Allocator>;
        using value_type = allocator_traits::value_type;
    };
}

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

        template<typename View>
        using src_allocations = src_allocations<allocator_type, View>;

        template<typename View>
        using src_callocations = src_callocations<allocator_type, View>;

        static constexpr auto size = std::ranges::size;

        static constexpr auto empty = std::ranges::empty;

        template<typename T = value_type>
        static constexpr auto data = allocation_data<Allocator, T>;

        template<typename T = value_type>
        static constexpr auto cdata = allocation_cdata<Allocator, T>;

        template<typename T = value_type>
        static constexpr auto get = allocation_get<Allocator, T>;

        template<typename T = value_type>
        static constexpr auto cget = allocation_cget<Allocator, T>;

        template<allocation<Allocator> Allocation>
        static constexpr Allocation
            allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr)
        {
            return Allocation{allocator_traits::allocate(alloc, size, hint), size};
        }

        template<allocation<Allocator> Allocation>
        static constexpr Allocation
            try_allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr) //
            noexcept
        {
            return Allocation{allocator_traits::try_allocate(alloc, size, hint), size};
        }

        template<typename View>
        static constexpr void deallocate(src_allocations<View> dst) noexcept
        {
            for(auto& dst_allocation : dst.allocations)
            {
                allocator_traits::deallocate(
                    dst.allocator,
                    data<>(dst_allocation),
                    size(dst_allocation)
                );
                dst_allocation = std::ranges::range_value_t<View>{};
            }
        }

        template<typename T = Allocator::value_type, typename View, typename... Args>
            requires(allocator_traits::template constructible_from<T, Args...>)
        static constexpr void construct(src_allocations<View> dst, Args&&... args) //
            noexcept(allocator_traits::template nothrow_constructible_from<T, Args...>)
        {
            for(const auto& dst_allocation : dst.allocations)
            {
                Expects(size(dst_allocation) * sizeof(value_type) >= sizeof(T));

                allocator_traits::construct(
                    dst.allocator,
                    data<T>(dst_allocation),
                    cpp_forward(args)...
                );
            }
        }

    private:
        template<typename Fn, typename Src, typename Dst>
        static constexpr auto allocation_ctor = std::copy_constructible<Fn> &&
            std::invocable<Fn&,
                           range_const_reference_t<Src>,
                           range_const_reference_t<Dst>,
                           allocator_type&>;

        template<typename Fn, typename Src, typename Dst>
        static constexpr auto nothrow_allocation_ctor = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&,
                              range_const_reference_t<Src>,
                              range_const_reference_t<Dst>,
                              allocator_type&>;

        template<typename Fn, typename Src, typename Dst>
        static constexpr auto allocation_assign = std::copy_constructible<Fn> &&
            std::invocable<Fn&, range_const_reference_t<Src>, range_const_reference_t<Dst>>;

        template<typename Fn, typename Src, typename Dst>
        static constexpr auto nothrow_allocation_assign = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&, range_const_reference_t<Src>, range_const_reference_t<Dst>>;

        template<typename Fn, typename Dst>
        static constexpr auto allocation_dtor = std::copy_constructible<Fn> &&
            std::invocable<Fn&, range_const_reference_t<Dst>, allocator_type&>;

        template<typename Fn, typename Dst>
        static constexpr auto nothrow_allocation_dtor = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&, range_const_reference_t<Dst>, allocator_type&>;

        template<typename View>
        static constexpr src_allocations<std::ranges::ref_view<View>>
            wrap(src_allocations<View>& src)
        {
            return {src.allocator, src.allocations};
        }

        template<typename View>
        static constexpr src_callocations<std::ranges::ref_view<View>>
            wrap(src_callocations<View>& src)
        {
            return {src.allocator, src.allocations};
        }

    public:
        template<typename View, typename Fn>
            requires allocation_dtor<Fn, View>
        static constexpr void destroy(src_allocations<View> dst, Fn fn) //
            noexcept(nothrow_allocation_dtor<Fn, View>)
        {
            auto& alloc = dst.allocator.get();
            for(const auto& allocation : dst.allocations) std::invoke(fn, allocation, alloc);
        }

    private:
        template<typename Src, typename Dst>
        static constexpr void mov_allocation(Src& src, Dst& dst) noexcept
        {
            for(auto&& [src_allocation, dst_allocation] : std::views::zip(src, dst))
            {
                dst_allocation =
                    std::ranges::range_value_t<Dst>{data<>(src_allocation), size(src_allocation)};
                src_allocation = std::ranges::range_value_t<Src>{};
            }
        }

    public:
        template<callocations_view<Allocator> Src, typename Dst, typename Fn>
            requires allocation_ctor<Fn, Src, Dst>
        static constexpr void on_construct(Src src, src_allocations<Dst> dst, Fn fn) //
            noexcept(nothrow_allocation_ctor<Fn, Src, Dst>)
        {
            for(const auto& [src_allocation, dst_allocation] :
                std::views::zip(src, dst.allocations))
                std::invoke(fn, src_allocation, dst_allocation, dst.allocator.get());
        }

        static constexpr void on_construct(
            allocations_view<Allocator> auto src,
            allocations_view<Allocator> auto dst
        ) noexcept
        {
            mov_allocation(src, dst);
        }

    private:
        static constexpr auto always_equal_v = allocator_traits::always_equal_v;

        static constexpr void
            validate_allocations_on_assign(const auto& src_allocation, const auto& dst_allocation)
        {
            Expects(!empty(src_allocation));
            Expects(!empty(dst_allocation));
        }

        static constexpr void value_cp_assign(auto& src, auto& dst, auto& fn)
        {
            for(const auto& [src_allocation, dst_allocation] : std::views::zip(src, dst))
            {
                validate_allocations_on_assign(src_allocation, dst_allocation);
                std::invoke(fn, src_allocation, dst_allocation);
            }
        }

    public:
        template<typename Src, typename Dst, typename Fn>
            requires allocation_assign<Fn, Src, Dst> && //
            allocator_traits::propagate_on_copy_v && //
            always_equal_v
        static constexpr void on_assign(
            src_callocations<Src> src,
            src_allocations<Dst> dst,
            Fn fn
        ) noexcept(nothrow_allocation_assign<Fn, Src, Dst>)
        {
            dst.allocator = src.allocator;
            value_cp_assign(src.allocations, dst.allocations, fn);
        }

        template<typename Src, typename Dst, typename Fn>
        static constexpr void on_assign(src_callocations<Src> src, src_allocations<Dst> dst, Fn fn)
            requires requires {
                requires allocator_traits::propagate_on_copy_v;
                requires allocation_assign<Fn, Src, Dst>;

                destroy(dst, fn);
                requires allocation_ctor<Fn, Src, Dst>;
            }
        {
            auto& src_alloc = src.allocator.get();
            auto& dst_alloc = dst.allocator.get();
            auto& src_allocations = src.allocations;
            auto& dst_allocations = dst.allocations;

            if(dst_alloc == src_alloc)
            {
                dst_alloc = src_alloc;
                value_cp_assign(src_allocations, dst_allocations, fn);
            }
            else
            {
                destroy(wrap(dst), std::ref(fn));
                deallocate(wrap(dst));

                dst_alloc = src_alloc;

                for(auto&& [src_allocation, dst_allocation] :
                    std::views::zip(src_allocations, dst_allocations))
                {
                    dst_allocation =
                        allocate<std::ranges::range_value_t<Dst>>(dst_alloc, src_allocation.size());
                    std::invoke(fn, src_allocation, dst_allocation, dst_alloc);
                }
            }
        }

        template<typename Src, typename Dst, typename Fn>
            requires allocation_assign<Fn, Src, Dst>
        static constexpr void on_assign(
            src_callocations<Src> src,
            src_allocations<Dst> dst,
            Fn fn
        ) noexcept(nothrow_allocation_assign<Fn, Src, Dst>)
        {
            value_cp_assign(src.allocations, dst.allocations, fn);
        }

    private:
        static constexpr void mov_allocation(auto& src, auto& dst, auto& fn)
        {
            destroy(wrap(dst), std::ref(fn));
            deallocate(wrap(dst));
            mov_allocation(src, dst);
        }

    public:
        template<typename Src, typename Dst, typename Fn>
        static constexpr void on_assign(
            src_allocations<Src> src,
            src_allocations<Dst> dst,
            Fn fn
        ) noexcept(nothrow_allocation_dtor<Fn, Dst>)
            requires allocator_traits::propagate_on_move_v && allocation_dtor<Fn, Dst>
        {
            mov_allocation(src.allocations, dst.allocations, fn);
            dst.allocator.get() = cpp_move(src.allocator.get());
        }

        template<typename Src, typename Dst, typename Fn>
        static constexpr void on_assign(
            src_allocations<Src> src,
            src_allocations<Dst> dst,
            Fn fn
        ) noexcept(nothrow_allocation_dtor<Fn, Dst>)
            requires requires {
                requires !allocator_traits::propagate_on_move_v;
                requires allocator_traits::always_equal_v;
                requires allocation_dtor<Fn, Dst>;
            }
        {
            mov_allocation(src.allocations, dst.allocations, fn);
        }

        template<typename Src, typename Dst, typename Fn>
        static constexpr void on_assign(
            src_allocations<Src> src,
            src_allocations<Dst> dst,
            Fn fn
        ) noexcept(nothrow_allocation_assign<Fn, Src, Dst>)
            requires allocation_assign<Fn, Src, Dst>
        {
            for(const auto& [src_allocation, dst_allocation] :
                std::views::zip(src.allocations, dst.allocations))
            {
                validate_allocations_on_assign(src_allocation, dst_allocation);
                std::invoke(fn, src_allocation, dst_allocation);
            }
        }

        template<typename Src, typename Dst>
        static constexpr void on_swap(src_allocations<Src> lhs, src_allocations<Dst> rhs) noexcept
        {
            if constexpr(allocator_traits::propagate_on_swap_v)
                std::ranges::swap(lhs.allocator.get(), rhs.allocator.get());
            else if constexpr(!always_equal_v) Expects(lhs.allocator.get() == rhs.allocator.get());

            for(auto&& [l, r] : std::views::zip(lhs.allocations, rhs.allocations))
            {
                using lhs_value_type = std::ranges::range_value_t<Src>;
                using rhs_value_type = std::ranges::range_value_t<Dst>;

                lhs_value_type l_tmp = cpp_move(l);
                l = lhs_value_type{cpp_move(r)};
                r = rhs_value_type{cpp_move(l_tmp)};
            }
        }
    };
}