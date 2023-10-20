#pragma once

#include "allocation_traits.h"
#include "../type_dispatchers.h"
#include <type_traits>

namespace stdsharp::allocator_aware
{
    template<allocation_req Allocation, typename T = Allocation::allocator_type::value_type>
    struct typed_allocation_traits : allocation_traits<Allocation>
    {
        using allocation_traits = allocation_traits<Allocation>;
        using typename allocation_traits::allocator_type;
        using typename allocation_traits::target;
        using typename allocation_traits::source;
        using typename allocation_traits::const_source;
        using typename allocation_traits::construction_result;
        using typename allocation_traits::allocation_cref;
        using allocator_traits = allocator_traits<allocator_type>;

        template<typename... Args>
            requires(allocator_traits::template constructible_from<T, Args...>)
        static constexpr T& construct(const target dst, Args&&... args) //
            noexcept(allocator_traits::template nothrow_constructible_from<T, Args...>)
        {
            auto& allocation = dst.allocation.get();
            Expects(allocation.size() >= sizeof(T));

            allocator_traits::construct(
                dst.allocator,
                allocation.template data<T>(),
                cpp_forward(args)...
            );

            return allocation.template get<T>();
        }

        static constexpr void destroy(const target dst) //
            noexcept(allocator_traits::template nothrow_destructible<T>)
            requires(allocator_traits::template destructible<T>)
        {
            auto& allocation = dst.allocation.get();
            if(allocation.empty()) return;
            allocator_traits::destroy(dst.allocator, allocation.template data<T>());
        }

        [[nodiscard]] static constexpr construction_result on_construct(const const_source src)
            requires(allocator_traits::template cp_constructible<T>)
        {
            auto& src_allocator = src.allocator;
            auto& src_allocation = src.allocation.get();
            construction_result result{
                allocation_traits::allocate(src_allocator, src_allocation.size()),
                allocator_traits::select_on_container_copy_construction(src_allocator)
            };
            construct({result.allocator, result.allocation}, src_allocation.template cget<T>());
            return result;
        }

        [[nodiscard]] static constexpr construction_result on_construct(const source src) noexcept
            requires std::move_constructible<T>
        {
            return {cpp_move(src.allocator.get()), std::exchange(src.allocation.get(), {})};
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
        static constexpr void on_assign(const const_source src, const target dst) //
            noexcept(nothrow_copy_assignable<T>)
            requires allocator_traits::propagate_on_copy_v && always_equal_v && copy_assignable<T>
        {
            auto& src_allocation = src.allocation.get();
            auto& dst_allocation = dst.allocation.get();

            validate_allocations_on_assign(src_allocation, dst_allocation);
            dst.allocator.get() = src.allocator.get();
            dst_allocation.template get<T>() = src_allocation.template cget<T>();
        }

        static constexpr void on_assign(const const_source src, const target dst)
            requires requires {
                requires allocator_traits::propagate_on_copy_v;
                requires copy_assignable<T>;
                destroy(dst);
                construct({dst.allocator, dst.allocation}, src.allocation.get().template cget<T>());
            }
        {
            auto& src_alloc = src.allocator.get();
            auto& dst_alloc = dst.allocator.get();
            auto& src_allocation = src.allocation.get();
            auto& dst_allocation = dst.allocation.get();

            validate_allocations_on_assign(src_allocation, dst_allocation);
            if(dst_alloc == src_alloc)
            {
                dst_alloc = src_alloc;
                dst_allocation.template get<T>() = src_allocation.template cget<T>();
            }
            else
            {
                destroy(dst);
                allocation_traits::deallocate(dst);
                dst_alloc = src_alloc;
                construct({dst.allocator, dst.allocation}, src_allocation.template cget<T>());
            }
        }

        static constexpr void on_assign(const const_source src, const target dst) //
            noexcept(nothrow_copy_assignable<T>)
            requires copy_assignable<T>
        {
            auto& src_allocation = src.allocation.get();
            auto& dst_allocation = dst.allocation.get();
            validate_allocations_on_assign(src_allocation, dst_allocation);
            dst_allocation.template get<T>() = src_allocation.template cget<T>();
        }

    private:
        static constexpr void mov_allocation(const source src, const target dst) //
            noexcept(noexcept(destroy(dst)))
            requires requires { destroy(dst); }
        {
            destroy(dst);
            allocation_traits::deallocate(dst);
            dst.allocation.get() = std::exchange(src.allocation.get(), {});
        }

    public:
        static constexpr void on_assign(const source src, const target dst) //
            noexcept(noexcept(mov_allocation(src, dst)))
            requires requires {
                requires allocator_traits::propagate_on_move_v;
                mov_allocation(src, dst);
            }
        {
            validate_allocations_on_assign(src.allocation, dst.allocation);
            mov_allocation(src, dst);
            dst.allocator.get() = cpp_move(src.allocator.get());
        }

        static constexpr void on_assign(const source src, const target dst) //
            noexcept(noexcept(mov_allocation(src, dst)))
            requires requires {
                requires !allocator_traits::propagate_on_move_v;
                requires allocator_traits::always_equal_v;
                requires move_assignable<T>;
                mov_allocation(src, dst);
            }
        {
            validate_allocations_on_assign(src.allocation, dst.allocation);
            mov_allocation(src, dst);
        }

        static constexpr void on_assign(const source src, const target dst) //
            noexcept(nothrow_move_assignable<T>)
            requires move_assignable<T>
        {
            validate_allocations_on_assign(src.allocation, dst.allocation);
            dst.allocation.get().template get<T>() =
                cpp_move(src.allocation.get().template get<T>());
        }

        static constexpr void on_swap(const source lhs, const target rhs) noexcept
            requires std::swappable<T>
        {
            if constexpr(allocator_traits::propagate_on_swap_v)
                std::swap(lhs.allocator.get(), rhs.allocator.get());
            else if constexpr(!always_equal_v) Expects(lhs.allocator.get() == rhs.allocator.get());
            std::swap(lhs.allocation.get(), rhs.allocation.get());
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

        template<special_mem_req Req>
        struct target : allocation_traits::target
        {
            type_dispatchers<Req, allocator_type> dispatchers;
        };

        template<special_mem_req Req>
        struct source : allocation_traits::source
        {
            type_dispatchers<Req, allocator_type> dispatchers;
        };

        template<special_mem_req Req>
        struct const_source : allocation_traits::const_source
        {
            type_dispatchers<Req, allocator_type> dispatchers;
        };

        template<special_mem_req Req>
        struct construction_result : allocation_traits::construction_result
        {
            type_dispatchers<Req, allocator_type> dispatchers;
        };

    private:
        template<typename T>
        using traits = typed_allocation_traits<Allocation, T>;

    public:
        template<typename T, special_mem_req Req, typename... Args>
            requires std::constructible_from<
                         type_dispatchers<Req, allocator_type>,
                         std::type_identity<T>> &&
            (allocator_traits::template constructible_from<T, Args...>)
        static constexpr T& construct(const target<Req> dst, Args&&... args) //
            noexcept(allocator_traits::template nothrow_constructible_from<T, Args...>)
        {
            auto& v = traits<T>::construct(dst, cpp_forward(args)...);
            dst.dispatchers = type_dispatchers<Req, allocator_type>{std::type_identity<T>{}};

            return v;
        }

        template<special_mem_req Req>
        static constexpr void destroy(const target<Req> dst) //
            noexcept(is_noexcept(Req.destruct))
            requires (is_well_formed(Req.destruct))
        {
            auto& allocation = dst.allocation.get();
            if(allocation.empty()) return;
            dst.dispatchers.destruct(allocation.data(), dst.allocator);
            dst.dispatchers = {};
        }

        template<special_mem_req Req>
        [[nodiscard]] static constexpr construction_result<Req> on_construct(const const_source<Req> src)
            requires requires(construction_result<Req> result) { traits::on_construct(src, result); }
        {
            auto& src_allocator = src.allocator;
            auto& src_allocation = src.allocation.get();
            construction_result<Req> result{
                allocation_traits::allocate(src_allocator, src_allocation.size()),
                allocator_traits::select_on_container_copy_construction(src_allocator),
                src.dispatchers
            };
            result.dispatchers.copy_construct(result.allocation, result.allocator, , src_allocation.template cget<T>());
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
    struct typed_allocation_traits<Allocation, void> : allocation_traits<Allocation>
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