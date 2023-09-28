#pragma once

#include "../allocator_aware.h"
#include "../../utility/dispatcher.h"

namespace stdsharp::details
{
    template<special_mem_req Req, typename Alloc>
    class box_dispatchers
    {
        using traits = allocator_traits<Alloc>;
        using alloc = traits::allocator_type;
        using alloc_cref = const alloc&;
        using allocation = allocator_aware::allocation<alloc>;
        using allocation_cref = const allocation&;

    public:
        using faked_typed_allocation = allocator_aware::typed_allocation<alloc, fake_type_for<Req>>;

        static constexpr auto req = faked_typed_allocation::operation_constraints;

    private:
        template<expr_req ExprReq, typename... Args>
        using ctor_dispatcher = dispatcher<ExprReq, allocation, alloc&, Args...>;

        template<expr_req ExprReq, typename... Args>
        using write_dispatcher =
            dispatcher<ExprReq, void, alloc&, allocation&, const bool, Args...>;

        using mov_ctor_dispatcher = ctor_dispatcher<req.move_construct, allocation&>;
        using cp_ctor_dispatcher = ctor_dispatcher<req.copy_construct, allocation_cref>;
        using mov_assign_dispatcher = write_dispatcher<req.move_assign, alloc&, allocation&>;
        using cp_assign_dispatcher = write_dispatcher<req.copy_assign, alloc_cref, allocation_cref>;
        using destroy_dispatcher = write_dispatcher<req.destruct>;
        using swap_dispatcher = write_dispatcher<req.swap, alloc&, allocation&>;

        using dispatchers = stdsharp::indexed_values<
            mov_ctor_dispatcher,
            cp_ctor_dispatcher,
            mov_assign_dispatcher,
            cp_assign_dispatcher,
            destroy_dispatcher,
            swap_dispatcher // clang-format off
        >; // clang-format on

        static constexpr dispatchers empty_dispatchers{};

        template<typename T, typename TypedAllocation = allocator_aware::typed_allocation<alloc, T>>
            requires(req <= TypedAllocation::operation_constraints)
        struct typed_dispatcher
        {
            static constexpr struct
            {
                constexpr auto operator()(alloc& alloc, allocation& other) const
                    noexcept(is_noexcept(req.move_construct))
                    requires faked_typed_allocation::mov_constructible
                {
                    TypedAllocation typed_allocation{other, true};
                    const auto res = typed_allocation.mov_construct(alloc);

                    other = typed_allocation.allocation();
                    return res.allocation();
                }
            } mov_construct{};

            static constexpr struct
            {
                constexpr auto operator()(alloc& alloc, allocation_cref other) const
                    noexcept(is_noexcept(req.copy_construct))
                    requires faked_typed_allocation::cp_constructible
                {
                    return TypedAllocation{other, true}.cp_construct(alloc).allocation();
                }
            } cp_construct{};

            static constexpr struct
            {
                constexpr void operator()(
                    alloc& dst_alloc,
                    allocation& dst_allocation,
                    const bool has_value,
                    alloc& src_alloc,
                    allocation& src_allocation
                ) const noexcept(is_noexcept(req.move_assign))
                    requires faked_typed_allocation::mov_assignable
                {
                    TypedAllocation dst{dst_allocation, has_value};
                    TypedAllocation src{src_allocation, true};

                    src.mov_assign(src_alloc, dst_alloc, dst);

                    dst_allocation = dst.allocation();
                    src_allocation = src.allocation();
                }
            } mov_assign{};

            static constexpr struct
            {
                constexpr void operator()(
                    alloc& dst_alloc,
                    allocation& dst_allocation,
                    const bool has_value,
                    alloc_cref src_alloc,
                    allocation_cref src_allocation
                ) const noexcept(is_noexcept(req.copy_assign))
                    requires faked_typed_allocation::cp_assignable
                {
                    TypedAllocation dst{dst_allocation, has_value};
                    TypedAllocation src{src_allocation, true};

                    src.cp_assign(src_alloc, dst_alloc, dst);
                    dst_allocation = dst.allocation();
                }
            } cp_assign{};

            static constexpr struct
            {
                constexpr void
                    operator()(alloc& alloc, allocation& allocation, const bool has_value) const
                    noexcept(is_noexcept(req.destruct))
                    requires faked_typed_allocation::destructible
                {
                    TypedAllocation src{allocation, has_value};
                    src.destroy(alloc);
                    allocation = src.allocation();
                }
            } destroy{};

            static constexpr struct
            {
                constexpr auto operator()(
                    alloc& dst_alloc,
                    allocation& dst_allocation,
                    const bool has_value,
                    alloc_cref src_alloc,
                    allocation_cref src_allocation
                ) const noexcept(is_noexcept(req.swap))
                    requires faked_typed_allocation::swappable
                {
                    TypedAllocation dst{dst_allocation, has_value};
                    TypedAllocation src{src_allocation, true};

                    src.swap(src_alloc, dst_alloc, dst);
                    dst_allocation = dst.allocation();
                }
            } swap{};

            static constexpr dispatchers
                dispatchers{mov_construct, cp_construct, mov_assign, cp_assign, destroy, swap};
        };

        constexpr box_dispatchers(const dispatchers& b) noexcept: dispatchers_(b) {}

    public:
        box_dispatchers() = default;

        template<typename T, typename TD = typed_dispatcher<T>>
        explicit constexpr box_dispatchers(const std::type_identity<T>) noexcept:
            box_dispatchers(TD::dispatchers)
        {
        }

        template<special_mem_req OtherReq>
        explicit constexpr box_dispatchers(const box_dispatchers<OtherReq, Alloc>& other) noexcept:
            box_dispatchers(other.dispatchers_)
        {
        }

        [[nodiscard]] constexpr auto construct(alloc& alloc, allocation& allocation) const
            noexcept(is_noexcept(req.copy_construct))
            requires faked_typed_allocation::cp_constructible
        {
            return cpo::get_element<0>(dispatchers_)(alloc, allocation);
        }

        [[nodiscard]] constexpr auto construct(alloc& alloc, allocation_cref allocation) const
            noexcept(is_noexcept(req.move_construct))
            requires faked_typed_allocation::mov_constructible
        {
            return cpo::get_element<1>(dispatchers_)(alloc, allocation);
        }

        constexpr void assign(
            alloc& dst_alloc,
            allocation& dst_allocation,
            const bool has_value,
            alloc& src_alloc,
            allocation& src_allocation
        ) const noexcept(is_noexcept(req.move_assign))
            requires faked_typed_allocation::mov_assignable
        {
            cpo::get_element<2>(dispatchers_)(
                dst_alloc,
                dst_allocation,
                has_value,
                src_alloc,
                src_allocation
            );
        }

        constexpr void assign(
            alloc& dst_alloc,
            allocation& dst_allocation,
            const bool has_value,
            alloc_cref src_alloc,
            allocation_cref src_allocation
        ) const noexcept(is_noexcept(req.copy_assign))
            requires faked_typed_allocation::cp_assignable
        {
            cpo::get_element<3>(dispatchers_)(
                dst_alloc,
                dst_allocation,
                has_value,
                src_alloc,
                src_allocation
            );
        }

        constexpr void destroy(alloc& alloc, allocation& allocation, const bool has_value) const
            noexcept(is_noexcept(req.destruct))
            requires faked_typed_allocation::destructible
        {
            cpo::get_element<4>(dispatchers_)(alloc, allocation, has_value);
        }

        constexpr auto do_swap(
            alloc& dst_alloc,
            allocation& dst_allocation,
            const bool has_value,
            alloc_cref src_alloc,
            allocation_cref src_allocation
        ) const noexcept(is_noexcept(req.swap))
            requires faked_typed_allocation::swappable
        {
            cpo::get_element<5>(dispatchers_)( // NOLINT(*-magic-numbers)
                dst_alloc,
                dst_allocation,
                has_value,
                src_alloc,
                src_allocation
            );
        }

        [[nodiscard]] constexpr auto has_value() const noexcept
        {
            return dispatchers_ == empty_dispatchers;
        }

        [[nodiscard]] constexpr operator bool() const noexcept { return has_value(); }

    private:
        dispatchers dispatchers_ = empty_dispatchers;
    };
}