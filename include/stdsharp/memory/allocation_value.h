#pragma once

#include "allocation_traits.h"
#include "../type_traits/special_member.h"
#include "../utility/dispatcher.h"
#include "launder_iterator.h"

namespace stdsharp
{
    template<allocator_req Allocator, typename T = Allocator::value_type>
    struct allocation_value
    {
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocator_type = allocation_traits::allocator_type;

    private:
        static constexpr auto element_size = sizeof(typename allocation_traits::value_type);

        static constexpr void size_validate(
            const callocation<Allocator> auto& src_allocation,
            const callocation<Allocator> auto& dst_allocation
        )
        {
            Expects(allocation_traits::size(src_allocation) * element_size >= value_size());
            Expects(allocation_traits::size(dst_allocation) * element_size >= value_size());
        }

    public:
        static constexpr auto always_equal = true;

        [[nodiscard]] static constexpr auto value_size() noexcept { return sizeof(T); }

        bool operator==(const allocation_value&) const = default;

        constexpr void operator()(
            allocator_type& allocator,
            const callocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(allocator_traits::template nothrow_cp_constructible<T>)
            requires(allocator_traits::template cp_constructible<T>)
        {
            size_validate(src_allocation, dst_allocation);

            allocator_traits::template construct<T>(
                allocator,
                allocation_traits::template data<T>(dst_allocation),
                allocation_traits::template cget<T>(src_allocation)
            );
        }

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(allocator_traits::template nothrow_mov_constructible<T>)
            requires(allocator_traits::template mov_constructible<T>)
        {
            size_validate(src_allocation, dst_allocation);

            allocator_traits::template construct<T>(
                allocator,
                allocation_traits::template data<T>(dst_allocation),
                cpp_move(allocation_traits::template get<T>(src_allocation))
            );
        }

        constexpr void operator()(
            const callocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation //
        ) const noexcept(nothrow_copy_assignable<T>)
            requires copy_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            allocation_traits::template get<T>(dst_allocation) =
                allocation_traits::template cget<T>(src_allocation);
        }

        constexpr void operator()(
            const allocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation //
        ) const noexcept(nothrow_move_assignable<T>)
            requires move_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            allocation_traits::template get<T>(dst_allocation) =
                cpp_move(allocation_traits::template get<T>(src_allocation));
        }

        constexpr void
            operator()(allocator_type& allocator, const allocation<Allocator> auto& allocation)
                const noexcept(allocator_traits::template nothrow_destructible<T>)
            requires(allocator_traits::template destructible<T>)
        {
            Expects(allocation_traits::size(allocation) * element_size >= value_size());
            allocator_traits::destroy(allocator, allocation_traits::template data<T>(allocation));
        }
    };

    template<allocator_req Allocator, typename T>
    class allocation_value<Allocator, T[]> // NOLINT(*-c-arrays)
    {
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocator_type = allocation_traits::allocator_type;
        using size_type = allocator_traits::size_type;

        static constexpr auto element_size = sizeof(allocation_traits::value_type);

        std::size_t size_;

        constexpr void size_validate(
            const callocation<Allocator> auto& src_allocation,
            const callocation<Allocator> auto& dst_allocation
        ) const noexcept
        {
            const auto size = value_size();

            Expects(allocation_traits::size(src_allocation) * element_size >= size);
            Expects(allocation_traits::size(dst_allocation) * element_size >= size);
        }

    public:
        constexpr explicit allocation_value(const size_type size) noexcept: size_(size) {}

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        [[nodiscard]] constexpr auto value_size() const noexcept { return size() * sizeof(T); }

        bool operator==(const allocation_value&) const = default;

        constexpr void operator()(
            allocator_type& allocator,
            const callocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(allocator_traits::template nothrow_cp_constructible<T>)
            requires(allocator_traits::template cp_constructible<T>)
        {
            size_validate(src_allocation, dst_allocation);
            for(std::size_t i = 0; i < size_; ++i)
                allocator_traits::template construct<T>(
                    allocator,
                    allocation_traits::template data<T>(dst_allocation) + i,
                    *std::launder(allocation_traits::template data<T>(src_allocation) + i)
                );
        }

        constexpr void operator()(
            allocator_type& allocator,
            const callocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(allocator_traits::template nothrow_mov_constructible<T>)
            requires(allocator_traits::template mov_constructible<T>)
        {
            size_validate(src_allocation, dst_allocation);
            for(std::size_t i = 0; i < size_; ++i)
                allocator_traits::template construct<T>(
                    allocator,
                    allocation_traits::template data<T>(dst_allocation) + i,
                    cpp_move(*std::launder(allocation_traits::template data<T>(src_allocation) + i))
                );
        }

        constexpr void operator()(
            const callocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation //
        ) const noexcept(nothrow_copy_assignable<T>)
            requires copy_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            std::ranges::copy_n(
                launder_iterator{allocation_traits::template cdata<T>(src_allocation)},
                size_,
                launder_iterator{allocation_traits::template data<T>(dst_allocation)}
            );
        }

        constexpr void operator()(
            const allocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation //
        ) const noexcept(nothrow_move_assignable<T>)
            requires move_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            std::ranges::copy_n(
                std::make_move_iterator(
                    launder_iterator{allocation_traits::template data<T>(src_allocation)}
                ),
                size_,
                launder_iterator{allocation_traits::template data<T>(dst_allocation)}
            );
        }

        constexpr void
            operator()(const allocation<Allocator> auto& allocation, allocator_type& allocator)
                const noexcept(allocator_traits::template nothrow_destructible<T>)
            requires(allocator_traits::template destructible<T>)
        {
            Expects(allocation.size() * element_size >= sizeof(T) * size_);

            for(size_type i = 0; i < size_; ++i)
                allocator_traits::destroy(
                    allocator,
                    std::launder(allocation_traits::template data<T>(allocation) + i)
                );
        }
    };
}

namespace stdsharp::details
{
    template<
        allocator_req Allocator,
        special_mem_req Req,
        typename Allocation,
        typename CAllocation>
    struct allocation_dynamic_value_operation
    {
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocator_type = allocation_traits::allocator_type;
        using fake_type = fake_type_for<Req>;

        using dispatchers = invocables<
            dispatcher<
                get_expr_req(
                    allocator_traits::template cp_constructible<fake_type>,
                    allocator_traits::template nothrow_cp_constructible<fake_type> //
                ),
                void,
                allocator_type&,
                const CAllocation&,
                const Allocation&>,
            dispatcher<
                get_expr_req(
                    allocator_traits::template mov_constructible<fake_type>,
                    allocator_traits::template nothrow_mov_constructible<fake_type> //
                ),
                void,
                allocator_type&,
                const Allocation&,
                const Allocation&>,
            dispatcher<
                get_expr_req(copy_assignable<fake_type>, nothrow_copy_assignable<fake_type>),
                void,
                const CAllocation&,
                const Allocation&>,
            dispatcher<
                get_expr_req(move_assignable<fake_type>, nothrow_move_assignable<fake_type>),
                void,
                const Allocation&,
                const Allocation&>,
            dispatcher<
                get_expr_req(
                    allocator_traits::template destructible<fake_type>,
                    allocator_traits::template nothrow_destructible<fake_type> //
                ),
                void,
                allocator_type&,
                const Allocation&>>;
    };
}

namespace stdsharp
{
    template<special_mem_req Req, typename, typename>
    struct allocation_dynamic_type : fake_type_for<Req>
    {
    };

    template<
        allocator_req Allocator,
        special_mem_req Req,
        allocation<Allocator> Allocation,
        callocation<Allocator> CAllocation>
    class allocation_value<Allocator, allocation_dynamic_type<Req, Allocation, CAllocation>> :
        details::allocation_dynamic_value_operation<Allocator, Req, Allocation, CAllocation>::
            dispatchers
    {
        using m_dispatchers =
            details::allocation_dynamic_value_operation<Allocator, Req, Allocation, CAllocation>::
                dispatchers;

        std::size_t value_size_{};

    public:
        using m_dispatchers::operator();

        static constexpr auto req = Req;

        allocation_value() = default;

        bool operator==(const allocation_value&) const = default;

        template<typename T, typename Op = allocation_value<T>>
            requires std::constructible_from<m_dispatchers, Op, Op, Op, Op, Op>
        explicit constexpr allocation_value(const std::type_identity<T> /*unused*/) noexcept:
            m_dispatchers(Op{}, Op{}, Op{}, Op{}, Op{}), value_size_(sizeof(T))
        {
        }

        template<
            special_mem_req OtherReq,
            typename Other = const allocation_value<
                Allocator,
                allocation_dynamic_type<OtherReq, Allocation, CAllocation>>&>
            requires std::constructible_from<m_dispatchers, Other, Other, Other, Other, Other> &&
                         (Req != OtherReq)
        explicit constexpr allocation_value(
            const allocation_value<
                Allocator,
                allocation_dynamic_type<OtherReq, Allocation, CAllocation>> other
        ) noexcept:
            m_dispatchers(other, other, other, other, other), value_size_(other.value_size_)
        {
        }

        [[nodiscard]] constexpr auto value_size() const noexcept { return value_size_; }
    };
}