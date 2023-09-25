#pragma once

#include "../allocator_aware.h"
#include "../../utility/implement_dispatcher.h"

namespace stdsharp::details
{
    template<special_mem_req Req, typename Alloc>
    class box_dispatchers
    {
        using traits = allocator_traits<Alloc>;
        using alloc = traits::allocator_type;
        using alloc_cref = const alloc&;
        using allocation = traits::allocation;
        using allocation_cref = const allocation&;
        using faked_typed_allocation = traits::template typed_allocation<fake_type_for<Req>>;

    public:
        static constexpr auto req = faked_typed_allocation::operation_constraints;

    private:
        template<expr_req ExprReq, typename... Args>
        using ctor_dispatcher = implement_dispatcher<ExprReq, allocation, alloc&, Args...>;

        template<expr_req ExprReq, typename... Args>
        using write_dispatcher =
            implement_dispatcher<ExprReq, void, alloc&, allocation&, const bool, Args...>;

        using mov_ctor_dispatcher = ctor_dispatcher<req.move_construct, allocation&>;
        using cp_ctor_dispatcher = ctor_dispatcher<req.copy_construct, allocation_cref>;
        using mov_assign_dispatcher = write_dispatcher<req.move_assign, alloc&, allocation&>;
        using cp_assign_dispatcher = write_dispatcher<req.copy_assign, alloc_cref, allocation_cref>;
        using destroy_dispatcher = write_dispatcher<expr_req::no_exception>;
        using swap_dispatcher = write_dispatcher<expr_req::no_exception, alloc&, allocation&>;

        using dispatchers = stdsharp::indexed_values<
            mov_ctor_dispatcher,
            cp_ctor_dispatcher,
            mov_assign_dispatcher,
            cp_assign_dispatcher,
            destroy_dispatcher,
            swap_dispatcher // clang-format off
        >; // clang-format on

        template<typename T, typename TypedAllocation = traits::template typed_allocation<T>>
            requires(req <= TypedAllocation::operation_constraints)
        struct typed_dispatcher
        {
            static constexpr struct
            {
                constexpr auto operator()(alloc& alloc, allocation& other) const
                    noexcept(req.move_construct >= expr_req::no_exception)
                    requires faked_typed_allocation::mov_constructible
                {
                    TypedAllocation typed_allocation{other, true};
                    const auto res = typed_allocation.mov_construct(alloc);

                    other = typed_allocation.allocation();
                    return res.allocation();
                }
            } mov_construct{};

            static constexpr struct : empty_t
            {
                constexpr auto operator()(alloc& alloc, allocation_cref other) const
                    noexcept(req.copy_construct >= expr_req::no_exception)
                    requires faked_typed_allocation::cp_constructible
                {
                    return TypedAllocation{other, true}.cp_construct(alloc).allocation();
                }
            } cp_construct{};

            static constexpr struct : empty_t
            {
                constexpr auto operator()(
                    alloc& dst_alloc,
                    allocation& dst_allocation,
                    const bool has_value,
                    alloc& src_alloc,
                    allocation& src_allocation
                ) const noexcept(req.move_assign >= expr_req::no_exception)
                    requires faked_typed_allocation::mov_assignable
                {
                    TypedAllocation dst{dst_allocation, has_value};
                    TypedAllocation src{src_allocation, true};

                    src.mov_assign(src_alloc, dst_alloc, dst, src_alloc);

                    dst_allocation = dst.allocation();
                    src_allocation = src.allocation();
                }
            } mov_assign{};

            static constexpr struct : empty_t
            {
                constexpr auto operator()(
                    alloc& dst_alloc,
                    allocation& dst_allocation,
                    const bool has_value,
                    alloc_cref src_alloc,
                    allocation_cref src_allocation
                ) const noexcept(req.copy_assign >= expr_req::no_exception)
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
                constexpr auto
                    operator()(alloc& alloc, allocation& allocation, const bool has_value) const
                    noexcept(req.destroy >= expr_req::no_exception)
                    requires faked_typed_allocation::destructible
                {
                    TypedAllocation src{allocation, has_value};
                    src.destroy(alloc);
                    allocation = src.allocation();
                }
            } destroy{};

            static constexpr struct : empty_t
            {
                constexpr auto operator()(
                    alloc& dst_alloc,
                    allocation& dst_allocation,
                    const bool has_value,
                    alloc_cref src_alloc,
                    allocation_cref src_allocation
                ) const noexcept(req.swap >= expr_req::no_exception)
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

        constexpr box_dispatchers(
            const dispatchers& b,
            const std::string_view current_type,
            const std::size_t type_size
        ) noexcept:
            dispatchers_(b), current_type_(current_type), type_size_(type_size)
        {
        }

    public:
        box_dispatchers() = default;

        template<typename T, typename TD = typed_dispatcher<std::decay_t<T>>>
        constexpr box_dispatchers(const std::type_identity<T>) noexcept:
            box_dispatchers(TD::dispatchers, type_id<T>, sizeof(T))
        {
        }

        template<special_mem_req OtherReq>
        constexpr box_dispatchers(const box_dispatchers<OtherReq, Alloc>& other) noexcept:
            box_dispatchers(other.dispatchers_, other.current_type_, other.type_size_)
        {
        }

        [[nodiscard]] constexpr auto construct(alloc& alloc, allocation& allocation) const
            noexcept(req.copy_construct == expr_req::no_exception)
            requires faked_typed_allocation::cp_constructible
        {
            return get<0>(dispatchers_)(alloc, allocation);
        }

        [[nodiscard]] constexpr auto construct(alloc& alloc, allocation_cref allocation) const
            noexcept(req.move_construct == expr_req::no_exception)
            requires faked_typed_allocation::mov_constructible
        {
            return get<1>(dispatchers_)(alloc, allocation);
        }

        constexpr void assign(
            alloc& dst_alloc,
            allocation& dst_allocation,
            const bool has_value,
            alloc& src_alloc,
            allocation& src_allocation
        ) const noexcept(req.move_assign == expr_req::no_exception)
            requires faked_typed_allocation::mov_assignable
        {
            get<2>(dispatchers_)(dst_alloc, dst_allocation, has_value, src_alloc, src_allocation);
        }

        constexpr void assign(
            alloc& dst_alloc,
            allocation& dst_allocation,
            const bool has_value,
            alloc_cref src_alloc,
            allocation_cref src_allocation
        ) const noexcept(req.copy_assign == expr_req::no_exception)
            requires faked_typed_allocation::cp_assignable
        {
            get<3>(dispatchers_)(dst_alloc, dst_allocation, has_value, src_alloc, src_allocation);
        }

        constexpr void destroy(alloc& alloc, allocation& allocation, const bool has_value) const
            noexcept(req.destroy >= expr_req::no_exception)
            requires faked_typed_allocation::destructible
        {
            get<4>(dispatchers_)(alloc, allocation, has_value);
        }

        constexpr auto do_swap(
            alloc& dst_alloc,
            allocation& dst_allocation,
            const bool has_value,
            alloc_cref src_alloc,
            allocation_cref src_allocation
        ) const noexcept(req.swap >= expr_req::no_exception)
            requires faked_typed_allocation::swappable
        {
            get<5>(dispatchers_)( // NOLINT(*-magic-numbers)
                dst_alloc,
                dst_allocation,
                has_value,
                src_alloc,
                src_allocation
            );
        }

        [[nodiscard]] constexpr auto type() const noexcept { return current_type_; }

        [[nodiscard]] constexpr auto size() const noexcept { return type_size_; }

        [[nodiscard]] constexpr auto has_value() const noexcept { return size() != 0; }

        [[nodiscard]] constexpr operator bool() const noexcept { return has_value(); }

    private:
        dispatchers dispatchers_{};
        std::string_view current_type_ = type_id<void>;
        std::size_t type_size_{};
    };
}