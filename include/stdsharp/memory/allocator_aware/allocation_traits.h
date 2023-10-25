#pragma once

#include "allocation.h"

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

        using allocation_type = allocation<allocator_type>;
        using callocation = callocation<allocation_type>;

        static constexpr allocation_type
            allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr)
        {
            return {allocator_traits::allocate(alloc, size, hint), size};
        }

        static constexpr allocation_type
            try_allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr) //
            noexcept
        {
            return {allocator_traits::try_allocate(alloc, size, hint), size};
        }

        template<typename View>
        static constexpr void deallocate( //
            const target_allocations<allocator_type, View> dst
        ) noexcept
        {
            for(allocation_type& allocation : dst.allocations)
            {
                allocator_traits::deallocate(dst.allocator, allocation.data(), allocation.size());
                allocation = {};
            }
        }

        template<typename T = Allocator::value_type, typename View, typename... Args>
            requires(allocator_traits::template constructible_from<T, Args...>)
        static constexpr void
            construct(const target_allocations<allocator_type, View> dst, Args&&... args) //
            noexcept(allocator_traits::template nothrow_constructible_from<T, Args...>)
        {
            for(allocation_type& allocation : dst.allocations)
            {
                Expects(allocation.size() * sizeof(value_type) >= sizeof(T));

                allocator_traits::construct(
                    dst.allocator,
                    allocation.template data<T>(),
                    cpp_forward(args)...
                );
            }
        }

    private:
        template<typename Fn>
        static constexpr auto copy_ctor = std::copy_constructible<Fn> &&
            std::invocable<Fn&, const callocation&, const allocation_type&, allocator_type&>;

        template<typename Fn>
        static constexpr auto copy_assign_op = std::copy_constructible<Fn> &&
            std::invocable<Fn&, const callocation&, const allocation_type&>;

        template<typename Fn>
        static constexpr auto move_assign_op = std::copy_constructible<Fn> &&
            std::invocable<Fn&, const allocation_type&, const allocation_type&>;

        template<typename Fn>
        static constexpr auto dtor = std::copy_constructible<Fn> &&
            std::invocable<Fn&, const allocation_type&, allocator_type&>;

        template<typename Fn>
        static constexpr auto nothrow_copy_ctor = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&, const callocation&, const allocation_type&, allocator_type&>;

        template<typename Fn>
        static constexpr auto nothrow_copy_assign_op = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&, const callocation&, const allocation_type&>;

        template<typename Fn>
        static constexpr auto nothrow_move_assign_op = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&, const allocation_type&, const allocation_type&>;

        template<typename Fn>
        static constexpr auto nothrow_dtor = nothrow_copy_constructible<Fn> &&
            nothrow_invocable<Fn&, const allocation_type&, allocator_type&>;

    public:
        template<typename View, typename Fn>
            requires dtor<Fn>
        static constexpr void destroy(const target_allocations<allocator_type, View> dst, Fn fn) //
            noexcept(nothrow_dtor<Fn>)
        {
            for(const allocation_type allocation : dst.allocations)
                std::invoke(fn, allocation, dst.allocator);
        }

        template<typename Allocations, typename Fn>
            requires copy_ctor<Fn>
        [[nodiscard]] static constexpr auto on_construct(
            const const_source_allocations<allocator_type, Allocations> src,
            Fn fn
        ) noexcept
        {
            return ctor_input_allocations{
                allocator_traits::select_on_container_copy_construction(src.allocator),
                std::bind_front(
                    [](const callocation src, Fn& fn, allocator_type& allocator) noexcept
                    {
                        return std::ranges::views::transform(
                            src,
                            std::bind_front(
                                [](allocator_type& allocator, Fn& fn, const auto src)
                                {
                                    auto allocation = allocate(allocator, src.size());
                                    std::invoke(fn, src, allocation, allocator);
                                    return allocation;
                                },
                                allocator,
                                fn
                            )
                        );
                    },
                    src.allocations,
                    cpp_move(fn)
                )
            };
        }

        template<typename Allocations>
        [[nodiscard]] static constexpr auto
            on_construct(const source_allocations<allocator_type, Allocations> src) noexcept
        {
            return ctor_input_allocations{
                cpp_move(src.allocator.get()),
                std::bind_front(
                    [](const auto src, auto&) noexcept { return src; },
                    std::ranges::views::transform(
                        src.allocations,
                        [](allocation_type& src) noexcept { return std::exchange(src, {}); }
                    )
                )
            };
        }

    private:
        static constexpr auto always_equal_v = allocator_traits::always_equal_v;

        static constexpr void
            validate_allocations_on_assign(callocation src_allocation, callocation dst_allocation)
        {
            Expects(!src_allocation.empty());
            Expects(!dst_allocation.empty());
        }

        static constexpr void value_cp_assign(const auto src, const auto dst, auto& fn)
        {
            for(auto&& [src_allocation, dst_allocation] : std::views::zip(src, dst))
            {
                validate_allocations_on_assign(src_allocation, dst_allocation);
                std::invoke(fn, src_allocation, dst_allocation);
            }
        }

    public:
        template<typename SrcView, typename TargetView, typename Fn>
            requires copy_assign_op<Fn> && //
            allocator_traits::propagate_on_copy_v && //
            always_equal_v
        static constexpr void on_assign(
            const const_source_allocations<allocator_type, SrcView> src,
            const target_allocations<allocator_type, TargetView> dst,
            Fn assign
        ) noexcept(nothrow_invocable<Fn, callocation, allocation_type>)
        {
            dst.allocator = src.allocator;
            value_cp_assign(
                src.allocations | views::cast<callocation>,
                dst.allocations | views::cast<allocation_type>,
                assign
            );
        }

        template<typename SrcView, typename TargetView, typename Fn>
        static constexpr void on_assign(
            const const_source_allocations<allocator_type, SrcView> src,
            const target_allocations<allocator_type, TargetView> dst,
            Fn fn
        )
            requires requires {
                requires allocator_traits::propagate_on_copy_v;
                requires copy_assign_op<Fn>;

                destroy(dst, fn);
                requires copy_ctor<Fn>;
            }
        {
            auto& src_alloc = src.allocator;
            auto& dst_alloc = dst.allocator;
            const auto src_allocations = src.allocations | views::cast<callocation>;
            const auto dst_allocations = dst.allocations | views::cast<allocation_type&>;

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
                    dst_allocation = allocate(dst_alloc, src_allocation.size());
                    std::invoke(fn, src_allocation, dst_allocation, dst_alloc);
                }
            }
        }

        template<typename SrcView, typename TargetView, typename Fn>
            requires copy_assign_op<Fn>
        static constexpr void on_assign(
            const const_source_allocations<allocator_type, SrcView> src,
            const target_allocations<allocator_type, TargetView> dst,
            Fn fn
        ) noexcept(nothrow_copy_assign_op<Fn>)
        {
            value_cp_assign(
                src.allocations | views::cast<callocation>,
                dst.allocations | views::cast<allocation_type&>,
                fn
            );
        }

    private:
        static constexpr void mov_allocation(const auto src, const auto dst, auto& fn) //
            noexcept(noexcept(destroy(dst, std::ref(fn))))
            requires requires { destroy(dst, std::ref(fn)); }
        {
            destroy(dst, std::ref(fn));
            deallocate(dst);

            for(auto& [src_allocation, dst_allocation] : std::views::zip(src, dst))
            {
                validate_allocations_on_assign(src_allocation, dst_allocation);
                dst_allocation = std::exchange(src_allocation, {});
            }
        }

    public:
        template<typename SrcView, typename TargetView, typename Fn>
        static constexpr void on_assign(
            const source_allocations<allocator_type, SrcView> src,
            const target_allocations<allocator_type, TargetView> dst,
            Fn fn
        ) noexcept( //
            noexcept( //
                mov_allocation(
                    src.allocations | views::cast<allocation_type&>,
                    dst.allocations | views::cast<allocation_type&>,
                    fn
                )
            )
        )
            requires requires {
                requires allocator_traits::propagate_on_move_v;
                mov_allocation(
                    src.allocations | views::cast<allocation_type&>,
                    dst.allocations | views::cast<allocation_type&>,
                    fn
                );
            }
        {
            mov_allocation(
                src.allocations | views::cast<allocation_type&>,
                dst.allocations | views::cast<allocation_type&>,
                fn
            );
            dst.allocator.get() = cpp_move(src.allocator.get());
        }

        template<typename SrcView, typename TargetView, typename Fn>
        static constexpr void on_assign(
            const source_allocations<allocator_type, SrcView> src,
            const target_allocations<allocator_type, TargetView> dst,
            Fn fn
        ) noexcept( //
            noexcept( //
                mov_allocation(
                    src.allocations | views::cast<allocation_type&>,
                    dst.allocations | views::cast<allocation_type&>,
                    fn
                )
            )
        )
            requires requires {
                requires !allocator_traits::propagate_on_move_v;
                requires allocator_traits::always_equal_v;
                mov_allocation(
                    src.allocations | views::cast<allocation_type&>,
                    dst.allocations | views::cast<allocation_type&>,
                    fn
                );
            }
        {
            mov_allocation(
                src.allocations | views::cast<allocation_type&>,
                dst.allocations | views::cast<allocation_type&>,
                fn
            );
        }

        template<typename SrcView, typename TargetView, typename Fn>
        static constexpr void on_assign(
            const source_allocations<allocator_type, SrcView> src,
            const target_allocations<allocator_type, TargetView> dst,
            Fn fn
        ) noexcept(nothrow_move_assign_op<Fn>)
            requires move_assign_op<Fn>
        {
            for(const auto [src_allocation, dst_allocation] : //
                std::views::zip(
                    src.allocations | views::cast<allocation_type>,
                    dst.allocations | views::cast<allocation_type> //
                ))
            {
                validate_allocations_on_assign(src_allocation, dst_allocation);
                std::invoke(fn, src_allocation, dst_allocation);
            }
        }

        template<typename SrcView, typename TargetView>
        static constexpr void on_swap(
            const source_allocations<allocator_type, SrcView> lhs,
            const target_allocations<allocator_type, TargetView> rhs
        ) noexcept
        {
            if constexpr(allocator_traits::propagate_on_swap_v)
                std::swap(lhs.allocator.get(), rhs.allocator.get());
            else if constexpr(!always_equal_v) Expects(lhs.allocator.get() == rhs.allocator.get());

            std::ranges::swap_ranges(
                lhs.allocations | views::cast<allocation_type&>,
                rhs.allocations | views::cast<allocation_type&> //
            );
        }
    };
}