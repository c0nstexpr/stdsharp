#pragma once

#include "allocation_traits.h"
#include "../type_dispatchers.h"
#include "../../utility/cast_to.h"

namespace stdsharp::allocator_aware
{
    template<typename Allocator>
    struct allocations_traits : allocation_traits<Allocator>
    {
        using allocation_traits = allocation_traits<Allocator>;
        using typename allocation_traits::allocator_type;
        using typename allocation_traits::allocator_cref;
        using typename allocation_traits::allocation_type;
        using typename allocation_traits::callocation;
        using allocator_traits = allocator_traits<allocator_type>;

    private:
        static constexpr auto allocations_transformer =
            std::ranges::views::transform(cast_to<allocation_type&>);

        static constexpr auto callocations_transformer =
            std::ranges::views::transform(cast_to<callocation>);

    public:
        template<typename T = Allocator::value_type, typename Allocations, typename... Args>
            requires(allocator_traits::template constructible_from<T, Args...>)
        static constexpr void
            construct(const target_allocations<allocator_type, Allocations> dst, Args&&... args) //
            noexcept(allocator_traits::template nothrow_constructible_from<T, Args...>)
        {
            for(auto& allocation : dst.allocations | allocations_transformer)
            {
                Expects(allocation.size() >= sizeof(T));

                allocator_traits::construct(
                    dst.allocator,
                    allocation.template data<T>(),
                    cpp_forward(args)...
                );
            }
        }

        template<typename T = Allocator::value_type, typename Allocations>
        static constexpr void destroy(const target_allocations<allocator_type, Allocations> dst) //
            noexcept(allocator_traits::template nothrow_destructible<T>)
            requires(allocator_traits::template destructible<T>)
        {
            for(auto& allocation : dst.allocations | allocations_transformer)
            {
                if(allocation.empty()) return;
                allocator_traits::destroy(dst.allocator, allocation.template data<T>());
            }
        }

    private:
        template<typename Fn>
        static constexpr auto copy_ctor = std::copy_constructible<Fn> &&
            std::invocable<Fn&, callocation, allocation_type&, allocator_type&>;

        template<typename Fn>
        static constexpr auto move_ctor = std::copy_constructible<Fn> &&
            std::invocable<Fn&, allocation_type&, allocation_type&, allocator_type&>;

    public:
        template<typename Allocations, std::copy_constructible CopyFn>
            requires std::invocable<CopyFn, callocation, allocation_type&, allocator_type&>
        [[nodiscard]] static constexpr auto on_construct(
            const const_source_allocations<allocator_type, Allocations> src,
            CopyFn&& fn
        ) noexcept
        {
            return ctor_input_allocation{
                allocator_traits::select_on_container_copy_construction(src.allocator),
                std::bind_front(
                    [](const auto src, CopyFn& fn, allocator_type& allocator) noexcept
                    {
                        return std::ranges::views::transform(
                            src,
                            std::bind_front(
                                [](allocator_type& allocator, CopyFn& fn, const auto src)
                                {
                                    auto allocation =
                                        allocation_traits::allocate(allocator, src.size());
                                    std::invoke(fn, src, allocation, allocator);
                                    return allocation;
                                },
                                allocator,
                                fn
                            )
                        );
                    },
                    src.allocations | callocations_transformer,
                    cpp_forward(fn)
                )
            };
        }

        template<typename T = Allocator::value_type>
        static constexpr nodiscard_invocable type_copy_constructor =
            []( //
                const callocation src_allocation,
                allocation_type& allocator,
                allocator_type& dst_allocation
            ) noexcept(allocator_traits::template nothrow_cp_constructible<T>)
            requires(allocator_traits::template cp_constructible<T>)
        {
            std::array<allocator_type&, 1> allocation{dst_allocation};
            construct<T>(
                {allocator, std::ranges::views::all(allocation)},
                src_allocation.template cget<T>()
            );
            dst_allocation = allocation.front();
        };

        template<typename T = Allocator::value_type, typename Allocations>
        [[nodiscard]] static constexpr auto
            on_construct(const const_source_allocations<allocator_type, Allocations> src) noexcept
            requires(allocator_traits::template cp_constructible<T>)
        {
            return on_construct(src, type_copy_constructor<T>);
        }

        template<typename Allocations>
        [[nodiscard]] static constexpr auto
            on_construct(const source_allocations<allocator_type, Allocations> src) noexcept
        {
            return ctor_input_allocation{
                cpp_move(src.allocator),
                std::bind_front(
                    [](const auto src, auto&) noexcept { return src; },
                    std::ranges::transform(
                        src.allocations | allocations_transformer,
                        [](auto& src) noexcept { return std::exchange(src, {}); }
                    )

                )
            };
        }

    private:
        static constexpr auto always_equal_v = allocator_traits::always_equal_v;

        static constexpr void validate_allocations_on_assign(
            callocation src_allocation, // NOLINT(*-swappable-parameters)
            callocation dst_allocation
        )
        {
            Expects(!src_allocation.empty());
            Expects(!dst_allocation.empty());
        }

        static constexpr void value_cp_assign(const auto src, const auto dst)
        {
            for(auto&& [src_allocation, dst_allocation] : std::views::zip(src, dst))
            {
                validate_allocations_on_assign(src_allocation, dst_allocation);
                dst_allocation.template get<T>() = src_allocation.template cget<T>();
            }
        }

    public:
        template<typename SrcAllocations, typename TargetAllocations>
        static constexpr void on_assign(
            const const_source_allocations<allocator_type, SrcAllocations> src,
            const target_allocations<allocator_type, TargetAllocations> dst
        ) //
            noexcept(nothrow_copy_assignable<T>)
            requires allocator_traits::propagate_on_copy_v && always_equal_v && copy_assignable<T>
        {
            dst.allocator = src.allocator;
            value_cp_assign(
                src.allocations | callocations_transformer,
                dst.allocations | allocations_transformer
            );
        }

        template<typename SrcAllocations, typename TargetAllocations>
        static constexpr void on_assign(
            const const_source_allocations<allocator_type, SrcAllocations> src,
            const target_allocations<allocator_type, TargetAllocations> dst
        )
            requires requires {
                requires allocator_traits::propagate_on_copy_v;
                requires copy_assignable<T>;
                destroy(dst);
                construct({dst.allocator, dst.allocation}, src.allocation.get().template cget<T>());
            }
        {
            auto& src_alloc = src.allocator;
            auto& dst_alloc = dst.allocator;
            const auto src_allocations = src.allocations | callocations_transformer;
            const auto dst_allocations = dst.allocations | allocations_transformer;

            if(dst_alloc == src_alloc)
            {
                dst_alloc = src_alloc;
                value_cp_assign(src_allocations, dst_allocations);
            }
            else
            {
                destroy(dst);
                allocation_traits::deallocate(dst);

                dst_alloc = src_alloc;

                for(auto&& [src_allocation, dst_allocation] :
                    std::views::zip(src_allocations, dst_allocations))
                {
                    std::array allocation{
                        allocation_traits::allocate(dst_alloc, src_allocation.size())
                    };
                    construct({dst_alloc, allocation}, src_allocation.template cget<T>());
                    dst_allocation = allocation;
                }
            }
        }

        template<typename SrcAllocations, typename TargetAllocations>
        static constexpr void on_assign(
            const const_source_allocations<allocator_type, SrcAllocations> src,
            const target_allocations<allocator_type, TargetAllocations> dst
        ) noexcept(nothrow_copy_assignable<T>)
            requires copy_assignable<T>
        {
            value_cp_assign(
                src.allocations | callocations_transformer,
                dst.allocations | allocations_transformer
            );
        }

    private:
        static constexpr void mov_allocation(const auto src, const auto dst) //
            noexcept(noexcept(destroy(dst)))
            requires requires { destroy(dst); }
        {
            destroy(dst);
            allocation_traits::deallocate(dst);

            for(auto& [src_allocation, dst_allocation] : std::views::zip(src, dst))
            {
                validate_allocations_on_assign(src_allocation, dst_allocation);
                dst.allocation = std::exchange(src.allocation, {});
            }
        }

    public:
        template<typename SrcAllocations, typename TargetAllocations>
        static constexpr void on_assign(
            const source_allocations<allocator_type, SrcAllocations> src,
            const target_allocations<allocator_type, TargetAllocations> dst
        ) noexcept(noexcept(mov_allocation(src, dst)))
            requires requires {
                requires allocator_traits::propagate_on_move_v;
                mov_allocation(src.allocations, dst.allocations);
            }
        {
            mov_allocation(
                src.allocations | allocations_transformer,
                dst.allocations | allocations_transformer
            );
            dst.allocator = cpp_move(src.allocator);
        }

        template<typename SrcAllocations, typename TargetAllocations>
        static constexpr void on_assign(
            const source_allocations<allocator_type, SrcAllocations> src,
            const target_allocations<allocator_type, TargetAllocations> dst
        ) noexcept(noexcept(mov_allocation(src, dst)))
            requires requires {
                requires !allocator_traits::propagate_on_move_v;
                requires allocator_traits::always_equal_v;
                requires move_assignable<T>;
                mov_allocation(src, dst);
            }
        {
            mov_allocation(
                src.allocations | allocations_transformer,
                dst.allocations | allocations_transformer
            );
        }

        template<typename SrcAllocations, typename TargetAllocations>
        static constexpr void on_assign(
            const source_allocations<allocator_type, SrcAllocations> src,
            const target_allocations<allocator_type, TargetAllocations> dst
        ) noexcept(nothrow_move_assignable<T>)
            requires move_assignable<T>
        {
            for(auto& [src_allocation, dst_allocation] : std::views::zip(
                    src.allocations | allocations_transformer,
                    dst.allocations | allocations_transformer
                ))
            {
                validate_allocations_on_assign(src_allocation, dst_allocation);
                dst_allocation.template get<T>() = cpp_move(src_allocation.template get<T>());
            }
        }

        template<typename SrcAllocations, typename TargetAllocations>
        static constexpr void on_swap(
            const source_allocations<allocator_type, SrcAllocations> lhs,
            const target_allocations<allocator_type, TargetAllocations> rhs
        ) noexcept
            requires std::swappable<T>
        {
            if constexpr(allocator_traits::propagate_on_swap_v)
                std::swap(lhs.allocator, rhs.allocator);
            else if constexpr(!always_equal_v) Expects(lhs.allocator == rhs.allocator);

            std::ranges::swap_ranges(
                lhs.allocations | allocations_transformer,
                rhs.allocations | allocations_transformer
            );
        }
    };

    template<allocation_req Allocation>
    struct typed_allocation_traits<Allocation, void> : allocation_traits<Allocation>
    {
        using allocation_traits = allocation_traits<Allocation>;
        using typename allocation_traits::allocation_type;
        using typename allocation_traits::callocation;
        using typename allocation_traits::allocator_traits;
        using typename allocation_traits::allocation;
        using typename allocation_traits::allocator_type;
        using typename allocation_traits::allocation_cref;
        using typename allocation_traits::allocator_cref;

        template<typename Allocations, special_mem_req Req>
        struct target : allocation_traits::template target<Allocations>
        {
            type_dispatchers<Req, allocator_type> dispatchers;

            template<typename T>
                requires std::constructible_from<Allocations, T>
            constexpr target(
                allocator_type& alloc,
                T&& allocations,
                const type_dispatchers<Req, allocator_type> dispatchers
            ) noexcept(nothrow_constructible_from<Allocations, T>):
                allocation_traits::template target<Allocations>(alloc, cpp_forward(allocations)),
                dispatchers(dispatchers)
            {
            }
        };

        template<typename Allocations, special_mem_req Req>
        struct source : allocation_traits::template source<Allocations>
        {
            type_dispatchers<Req, allocator_type> dispatchers;
        };

        template<typename Allocations, special_mem_req Req>
        struct const_source : allocation_traits::template const_source<Allocations>
        {
            type_dispatchers<Req, allocator_type> dispatchers;
        };

        template<typename Allocations, special_mem_req Req>
        struct construction_result : allocation_traits::template construction_result<Allocations>
        {
            type_dispatchers<Req, allocator_type> dispatchers;
        };

    private:
        template<typename T>
        using traits = typed_allocation_traits<Allocation, T>;

    public:
        template<typename T, special_mem_req Req, typename Allocations, typename... Args>
            requires std::constructible_from<
                         type_dispatchers<Req, allocator_type>,
                         std::type_identity<T>> &&
            (allocator_traits::template constructible_from<T, Args...>)
        static constexpr T& construct(const target<Req, Allocations> dst, Args&&... args) //
            noexcept(allocator_traits::template nothrow_constructible_from<T, Args...>)
        {
            auto& v = traits<T>::construct(dst, cpp_forward(args)...);
            dst.dispatchers = type_dispatchers<Req, allocator_type>{std::type_identity<T>{}};

            return v;
        }

        template<special_mem_req Req, typename Allocations>
        static constexpr void destroy(const target<Req, Allocations> dst) //
            noexcept(is_noexcept(Req.destruct))
            requires(is_well_formed(Req.destruct))
        {
            auto& allocation = dst.allocation.get();
            if(allocation.empty()) return;
            dst.dispatchers.destruct(allocation.data(), dst.allocator);
            dst.dispatchers = {};
        }

        template<special_mem_req Req, typename Allocations>
        [[nodiscard]] static constexpr construction_result<Req, Allocations>
            on_construct(const const_source<Req, Allocations> src)
            requires requires(construction_result<Req, Allocations> result) {
                traits::on_construct(src, result);
            }
        {
            auto& src_allocator = src.allocator;
            auto& src_allocation = src.allocation.get();
            construction_result<Req> result{
                allocation_traits::allocate(src_allocator, src_allocation.size()),
                allocator_traits::select_on_container_copy_construction(src_allocator),
                src.dispatchers
            };
            result.dispatchers.copy_construct(
                result.allocation,
                result.allocator,
                ,
                src_allocation.template cget<T>()
            );
            return result;
        }

        [[nodiscard]] static constexpr allocation_type on_construct(const source src) //
            noexcept(
                noexcept(allocation_type::on_construct(src, std::declval<construction_result&>()))
            )
            requires requires(construction_result result) {
                allocation_type::on_construct(src, result);
            }
        {
            construction_result result{{}, cpp_move(src.allocator.get())};
            allocation_type::on_construct(src, result);
            return result;
        }

        static constexpr void has_value(const callocation& allocation) noexcept
        {
            return allocation.size() != 0;
        }

    private:
        static constexpr void do_assign(const auto src, const target dst) noexcept(
            noexcept(allocation_type::on_assign(src, dst))
        )
            requires requires { allocation_type::on_assign(src, dst); }
        {
            allocation_type::on_assign(src, dst);
            dst.allocation.get().type() = src.allocation.get().type();
        }

        static constexpr void prepare_for(const auto src, const target dst)
            requires requires { destroy(dst); }
        {
            auto& src_allocation = src.allocation.get();
            auto& dst_allocation = dst.allocation.get();

            if(allocation_type::is_assignable(src_allocation, dst_allocation)) return;

            auto& size = src_allocation.size();

            destroy(dst);
            if(dst_allocation.capcity() < size())
            {
                allocation_traits::deallocate(dst);
                dst_allocation = allocation_traits::allocate(src.allocator, size);
            }
        }

    public:
    };

    template<typename Allocation>
    struct allocations_traits<Allocation, void> : allocation_traits<Allocation>
    {
        using allocation_traits = allocation_traits<Allocation>;
        using typename allocation_traits::allocator_traits;
        using typename allocation_traits::allocation_type;
        using typename allocation_traits::callocation;
        using typename allocation_traits::allocator_type;
        using typename allocation_traits::allocation_cref;
        using typename allocation_traits::allocator_cref;
        using typename allocation_traits::target;
        using typename allocation_traits::source;
        using typename allocation_traits::const_source;
        using typename allocation_traits::construction_result;
    };
}