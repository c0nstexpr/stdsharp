#pragma once

#include "box_dispatchers.h"
#include "../allocator_aware/allocation_traits.h"

namespace stdsharp::details
{
    template<special_mem_req Req, allocator_req Alloc>
    class box_allocation
    {
        template<special_mem_req, allocator_req>
        friend class box_allocation;

    public:
        using allocator_type = Alloc;

    private:
        using traits = allocator_traits<allocator_type>;
        using dispatchers = details::box_dispatchers<Req, allocator_type>;
        using faked_typed = dispatchers::faked_typed_allocation;
        using allocation = traits::allocation;
        using size_type = traits::size_type;
        using cvp = traits::const_void_pointer;

        static constexpr auto mov_allocation_v =
            traits::propagate_on_move_v || traits::always_equal_v;

        dispatchers dispatchers_{};
        allocation allocation_{};

    public:
        static constexpr auto req = dispatchers::req;

        constexpr box_allocation cp_construct(allocator_type& alloc) //
            noexcept(is_noexcept(req.copy_construct))
            requires faked_typed::cp_constructible
        {
            return dispatchers_ ? dispatchers_.construct(alloc, allocation_) : allocation{};
        }

        constexpr box_allocation mov_construct(allocator_type& alloc) //
            noexcept(is_noexcept(req.move_construct))
            requires faked_typed::mov_constructible
        {
            return dispatchers_ ? dispatchers_.construct(alloc, cpp_move(allocation_)) :
                                  allocation{};
        }

    private:
        constexpr void do_assign(auto& src_alloc, allocator_type& dst_alloc, auto& dst_allocation)
        {
            auto& dst_dispatchers = dst_allocation.dispatchers_;

            dispatchers_.assign(
                src_alloc,
                allocation_,
                dst_alloc,
                dst_allocation.allocation_,
                dst_dispatchers.has_value()
            );

            dst_dispatchers = dispatchers_;
        }

    public:
        template<special_mem_req DstReq>
            requires(box_allocation<DstReq, allocator_type>::req <= req)
        constexpr void cp_assign(
            const allocator_type& src_alloc,
            allocator_type& dst_alloc,
            box_allocation<DstReq, allocator_type>& dst_allocation
        )
            requires faked_typed::destructible && faked_typed::cp_assignable
        {
            auto& dst_dispatchers = dst_allocation.dispatchers_;

            if(dispatchers_ != dst_dispatchers)
            {
                dst_allocation.destroy();
                dst_allocation.allocate(dispatchers_.type_size());
            }

            do_assign(src_alloc, dst_alloc, dst_allocation);
        }

        template<special_mem_req DstReq>
            requires(box_allocation<DstReq, allocator_type>::req <= req)
        constexpr void mov_assign(
            const allocator_type& src_alloc,
            allocator_type& dst_alloc,
            box_allocation<DstReq, allocator_type>& dst_allocation
        )
            requires faked_typed::destructible && faked_typed::mov_assignable && (!mov_allocation_v)
        {
            auto& dst_dispatchers = dst_allocation.dispatchers_;

            if(src_alloc == dst_alloc) dst_allocation.destroy();
            else if(dispatchers_ != dst_dispatchers)
            {
                dst_allocation.destroy();
                dst_allocation.allocate(dst_dispatchers.type_size());
            }

            do_assign(src_alloc, dst_alloc, dst_allocation);
        }

        template<special_mem_req DstReq>
            requires(box_allocation<DstReq, allocator_type>::req <= req)
        constexpr void mov_assign(
            const allocator_type& src_alloc,
            allocator_type& dst_alloc,
            box_allocation<DstReq, allocator_type>& dst_allocation
        ) noexcept
            requires faked_typed::destructible && mov_allocation_v
        {
            destroy();
            do_assign(src_alloc, dst_alloc, dst_allocation);
        }

        template<special_mem_req DstReq>
        constexpr void swap(
            const allocator_type& src_alloc,
            allocator_type& dst_alloc,
            box_allocation<DstReq, allocator_type>& dst
        ) noexcept
        {
            allocator_aware::allocation_traits<allocation>::swap(
                allocation_,
                src_alloc,
                dst.allocation_,
                dst_alloc
            );
        }

        constexpr void
            allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr)
        {
            allocation_.allocate(alloc, size, hint);
        }

        constexpr void deallocate(allocator_type& alloc) { allocation_.deallocate(alloc); }

        template<typename T>
        constexpr T& construct(allocator_type& alloc, auto&&... args)
            requires requires(allocator_aware::typed_allocation<allocator_type, T> allocation) {
                allocation.construct(alloc, cpp_forward(args)...);
                requires std::constructible_from<dispatchers, std::type_identity<T>>;
                requires faked_typed::destructible;
            }
        {
            destroy();
            allocate(alloc, sizeof(T));

            allocator_aware::typed_allocation<allocator_type, T> allocation{allocation_};

            allocation.construct(alloc, cpp_forward(args)...);
            dispatchers_ = dispatchers{std::type_identity<T>{}};

            return allocation.get();
        }

        constexpr void destroy(allocator_type& alloc) noexcept(is_noexcept(req.destruct))
            requires faked_typed::destructible
        {
            auto& dispatchers = dispatchers_;

            if(!dispatchers) return;

            dispatchers.destroy(alloc, allocation_, true);
            dispatchers = {};
        }

        template<typename T>
        [[nodiscard]] constexpr T& get() const noexcept
        {
            return *pointer_cast<T>(allocation_.begin());
        }

        [[nodiscard]] constexpr bool empty() const noexcept { return dispatchers_; }

        template<typename T>
        [[nodiscard]] constexpr auto is_type() const noexcept
        {
            if constexpr(std::constructible_from<dispatchers, std::type_identity<T>>)
                return dispatchers_ == dispatchers{std::type_identity<T>{}};
            else return false;
        }

        [[nodiscard]] constexpr auto size() const noexcept { return allocation_.size(); }
    };
}