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
        static constexpr void deallocate( //
            const source_allocations<allocator_type, View> dst
        ) noexcept
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
        static constexpr void
            construct(const source_allocations<allocator_type, View> dst, Args&&... args) //
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
                           std::ranges::range_reference_t<Dst>,
                           allocator_type&>;

        template<typename Fn, typename Src, typename Dst>
        static constexpr auto nothrow_allocation_ctor = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&,
                              range_const_reference_t<Src>,
                              std::ranges::range_reference_t<Dst>,
                              allocator_type&>;

        template<typename Fn, typename Src, typename Dst>
        static constexpr auto allocation_assign_op = std::copy_constructible<Fn> &&
            std::invocable<Fn&, range_const_reference_t<Src>, std::ranges::range_reference_t<Dst>>;

        template<typename Fn, typename Src, typename Dst>
        static constexpr auto nothrow_allocation_assign_op = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&,
                              range_const_reference_t<Src>,
                              std::ranges::range_reference_t<Dst>>;

        template<typename Fn, typename Dst>
        static constexpr auto allocation_dtor = std::copy_constructible<Fn> &&
            std::invocable<Fn&, std::ranges::range_reference_t<Dst>, allocator_type&>;

        template<typename Fn, typename Dst>
        static constexpr auto nothrow_allocation_dtor = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&, std::ranges::range_reference_t<Dst>, allocator_type&>;

    public:
        template<typename View, typename Fn>
            requires allocation_dtor<Fn, View>
        static constexpr void destroy(const source_allocations<allocator_type, View> dst, Fn fn) //
            noexcept(nothrow_allocation_dtor<Fn, View>)
        {
            auto& alloc = dst.allocator.get();
            for(const auto& allocation : dst.allocations) std::invoke(fn, allocation, alloc);
        }

        template<typename Src, typename Fn>
            requires std::copy_constructible<Fn>
        [[nodiscard]] static constexpr auto on_construct(
            const const_source_allocations<allocator_type, Src> src,
            Fn fn
        ) noexcept(nothrow_move_constructible<Fn>)
        {
            return defer_allocations{
                std::bind_front(
                    [](const auto& alloc) noexcept
                    {
                        return allocator_traits::select_on_container_copy_construction(alloc); //
                    },
                    src.allocator
                ),
                std::bind_front(
                    []<allocations_view<allocator_type> Dst>(
                        const auto src,
                        auto& fn,
                        const Dst dst,
                        allocator_type& allocator
                    )
                        requires allocation_ctor<Fn, Src, Dst>
                    {
                        for(auto&& [src_allocation, dst_allocation] : std::views::zip(src, dst))
                        {
                            dst_allocation = allocate<std::ranges::range_value_t<Dst>>(
                                allocator,
                                size(src_allocation)
                            );
                            std::invoke(fn, src_allocation, dst_allocation, allocator);
                        }
                    },
                    src.allocations,
                    cpp_move(fn)
                )
                };
        }

    private:
        struct mov_allocation_fn
        {
            template<allocations_view<allocator_type> Src, allocations_view<allocator_type> Dst>
            constexpr void
                operator()(const Src src, const Dst dst, const auto&... /*unused*/) const noexcept
            {
                std::ranges::move(
                    std::views::transform(
                        src,
                        [](auto& allocation)
                        {
                            return std::ranges::range_value_t<Dst>{
                                std::exchange(allocation, std::ranges::range_value_t<Src>{})
                            };
                        }
                    ),
                    std::ranges::begin(dst)
                );
            }
        };

    public:
        template<typename Src>
        [[nodiscard]] static constexpr auto
            on_construct(const source_allocations<allocator_type, Src> src) noexcept
        {
            return defer_allocations{
                std::bind_front(
                    [](auto&& alloc) noexcept { return cpp_move(alloc.get()); },
                    src.allocator
                ),
                std::bind_front(mov_allocation_fn{}, src.allocations)
            };
        }

    private:
        static constexpr auto always_equal_v = allocator_traits::always_equal_v;

        static constexpr void
            validate_allocations_on_assign(const auto& src_allocation, const auto& dst_allocation)
        {
            Expects(!empty(src_allocation));
            Expects(!empty(dst_allocation));
        }

        static constexpr void value_cp_assign(const auto src, const auto dst, auto& fn)
        {
            for(const auto& [src_allocation, dst_allocation] : std::views::zip(src, dst))
            {
                validate_allocations_on_assign(src_allocation, dst_allocation);
                std::invoke(fn, src_allocation, dst_allocation);
            }
        }

    public:
        template<typename Src, typename Dst, typename Fn>
            requires allocation_assign_op<Fn, Src, Dst> && //
            allocator_traits::propagate_on_copy_v && //
            always_equal_v
        static constexpr void on_assign(
            const const_source_allocations<allocator_type, Src> src,
            const source_allocations<allocator_type, Dst> dst,
            Fn assign
        ) noexcept(nothrow_allocation_assign_op<Fn, Src, Dst>)
        {
            dst.allocator = src.allocator;
            value_cp_assign(src.allocations, dst.allocations, assign);
        }

        template<typename Src, typename Dst, typename Fn>
        static constexpr void on_assign(
            const const_source_allocations<allocator_type, Src> src,
            const source_allocations<allocator_type, Dst> dst,
            Fn fn
        )
            requires requires {
                requires allocator_traits::propagate_on_copy_v;
                requires allocation_assign_op<Fn, Src, Dst>;

                destroy(dst, fn);
                requires allocation_ctor<Fn, Src, Dst>;
            }
        {
            auto& src_alloc = src.allocator;
            auto& dst_alloc = dst.allocator;
            const auto src_allocations = src.allocations;
            const auto dst_allocations = dst.allocations;

            if(dst_alloc == src_alloc)
            {
                dst_alloc = src_alloc;
                value_cp_assign(src_allocations, dst_allocations, fn);
            }
            else
            {
                destroy(dst, std::ref(fn));
                deallocate(dst);

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
            requires allocation_assign_op<Fn, Src, Dst>
        static constexpr void on_assign(
            const const_source_allocations<allocator_type, Src> src,
            const source_allocations<allocator_type, Dst> dst,
            Fn fn
        ) noexcept(nothrow_allocation_assign_op<Fn, Src, Dst>)
        {
            value_cp_assign(src.allocations, dst.allocations, fn);
        }

    private:
        static constexpr void mov_allocation(const auto src, const auto dst, auto& fn)
        {
            destroy(dst, std::ref(fn));
            deallocate(dst);
            mov_allocation_fn{}(src, dst);
        }

    public:
        template<typename Src, typename Dst, typename Fn>
        static constexpr void on_assign(
            const source_allocations<allocator_type, Src> src,
            const source_allocations<allocator_type, Dst> dst,
            Fn fn
        ) noexcept(nothrow_allocation_dtor<Fn, Dst>)
            requires allocator_traits::propagate_on_move_v && allocation_dtor<Fn, Dst>
        {
            mov_allocation(src.allocations, dst.allocations, fn);
            dst.allocator.get() = cpp_move(src.allocator.get());
        }

        template<typename Src, typename Dst, typename Fn>
        static constexpr void on_assign(
            const source_allocations<allocator_type, Src> src,
            const source_allocations<allocator_type, Dst> dst,
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
            const source_allocations<allocator_type, Src> src,
            const source_allocations<allocator_type, Dst> dst,
            Fn fn
        ) noexcept(nothrow_allocation_assign_op<Fn, Src, Dst>)
            requires allocation_assign_op<Fn, Src, Dst>
        {
            for(const auto [src_allocation, dst_allocation] :
                std::views::zip(src.allocations, dst.allocations))
            {
                validate_allocations_on_assign(src_allocation, dst_allocation);
                std::invoke(fn, src_allocation, dst_allocation);
            }
        }

        template<typename Src, typename Dst>
        static constexpr void on_swap(
            const source_allocations<allocator_type, Src> lhs,
            const source_allocations<allocator_type, Dst> rhs
        ) noexcept
        {
            if constexpr(allocator_traits::propagate_on_swap_v)
                std::ranges::swap(lhs.allocator.get(), rhs.allocator.get());
            else if constexpr(!always_equal_v) Expects(lhs.allocator.get() == rhs.allocator.get());

            for(auto& [l, r] : std::views::zip(lhs.allocations, rhs.allocations))
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