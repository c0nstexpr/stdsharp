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
        using allocation = allocator_aware::allocation<allocator_type>;
        using allocation_traits = allocator_aware::allocation_traits<allocation>;
        using size_type = traits::size_type;
        using cvp = traits::const_void_pointer;

        static constexpr auto mov_allocation_v =
            traits::propagate_on_move_v || traits::always_equal_v;

        dispatchers dispatchers_{};
        allocation allocation_{};

    public:
        static constexpr auto req = dispatchers::req;

        box_allocation() = default;

        constexpr box_allocation(const dispatchers dispatchers, const allocation allocation) //
            noexcept:
            dispatchers_{dispatchers}, allocation_{allocation}
        {
        }

    private:
        template<special_mem_req DstReq>
        static constexpr auto is_compatible = box_allocation<DstReq, allocator_type>::req <= req;

    public:
        template<special_mem_req DstReq>
            requires is_compatible<DstReq>
        constexpr box_allocation<DstReq, allocator_type> cp_construct(allocator_type& alloc) //
            noexcept(is_noexcept(req.copy_construct))
            requires faked_typed::cp_constructible
        {
            return {
                dispatchers_,
                dispatchers_ ? dispatchers_.construct(alloc, allocation_) : allocation{}
            };
        }

        template<special_mem_req DstReq>
            requires is_compatible<DstReq>
        constexpr box_allocation<DstReq, allocator_type> mov_construct(allocator_type& alloc) //
            noexcept(is_noexcept(req.move_construct))
            requires faked_typed::mov_constructible
        {
            return {
                dispatchers_,
                dispatchers_ ? dispatchers_.construct(alloc, cpp_move(allocation_)) : allocation{}
            };
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

        constexpr void prepare_for(allocator_type& dst_alloc, auto& dst_allocation) const
        {
            if(dispatchers_ != dst_allocation.dispatchers_)
            {
                dst_allocation.destroy(dst_alloc);
                if(dst_allocation.size() < dispatchers_.type_size())
                    dst_allocation.allocate(dst_alloc, dispatchers_.type_size());
            }
        }

    public:
        constexpr void cp_assign(
            const allocator_type& src_alloc,
            allocator_type& dst_alloc,
            box_allocation& dst_allocation
        )
            requires faked_typed::destructible && faked_typed::cp_assignable
        {
            prepare_for(dst_alloc, dst_allocation);
            do_assign(src_alloc, dst_alloc, dst_allocation);
        }

        constexpr void mov_assign(
            allocator_type& src_alloc,
            allocator_type& dst_alloc,
            box_allocation& dst_allocation
        )
            requires faked_typed::destructible && faked_typed::mov_assignable && (!mov_allocation_v)
        {
            prepare_for(dst_alloc, dst_allocation);
            do_assign(src_alloc, dst_alloc, dst_allocation);
        }

        constexpr void mov_assign(
            allocator_type& src_alloc,
            allocator_type& dst_alloc,
            box_allocation& dst_allocation
        ) noexcept
            requires faked_typed::destructible && mov_allocation_v
        {
            dst_allocation.destroy(dst_alloc);
            do_assign(src_alloc, dst_alloc, dst_allocation);
        }

        constexpr void
            swap(allocator_type& src_alloc, allocator_type& dst_alloc, box_allocation& dst) noexcept
        {
            allocation_traits::swap(allocation_, src_alloc, dst.allocation_, dst_alloc);
        }

        constexpr void
            allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr)
        {
            allocation_.allocate(alloc, size, hint);
        }

        constexpr void deallocate(allocator_type& alloc) noexcept(is_noexcept(req.destruct))
            requires faked_typed::destructible
        {
            destroy(alloc);
            allocation_.deallocate(alloc);
        }

        template<typename T>
        constexpr T& construct(allocator_type& alloc, auto&&... args)
            requires requires(allocator_aware::typed_allocation<allocator_type, T> allocation) {
                allocation.construct(alloc, cpp_forward(args)...);
                requires std::constructible_from<dispatchers, std::type_identity<T>>;
                requires faked_typed::destructible;
            }
        {
            destroy(alloc);
            allocate(alloc, sizeof(T));

            T& value =
                allocation_traits::template construct<T>(allocation_, alloc, cpp_forward(args)...);

            dispatchers_ = dispatchers{std::type_identity<T>{}};

            return value;
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

        [[nodiscard]] constexpr auto type_size() const noexcept { return dispatchers_.type_size(); }
    };
}