#pragma once

#include "allocation_traits.h"
#include "../../type_traits/special_member.h"
#include "../../utility/dispatcher.h"
#include "../launder_iterator.h"

namespace stdsharp::allocator_aware
{
    template<allocator_req Allocator, typename T = Allocator::value_type>
    struct allocation_value
    {
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocation_type = allocation_traits::allocation_type;
        using allocator_type = allocation_traits::allocator_type;
        using callocation = allocation_traits::callocation;

    private:
        static constexpr void size_validate(
            const callocation src_allocation,
            const callocation dst_allocation
        ) noexcept
        {
            constexpr auto value_type_size = sizeof(Allocator::value_type);

            Expects(std::ranges::size(src_allocation) * value_type_size >= sizeof(T));
            Expects(std::ranges::size(dst_allocation) * value_type_size >= sizeof(T));
        }

    public:
        static constexpr auto always_equal = true;

        bool operator==(const allocation_value&) const = default;

        constexpr void operator()(
            const callocation src_allocation,
            const allocation_type dst_allocation,
            allocator_type& allocator
        ) const noexcept(allocator_traits::template nothrow_cp_constructible<T>)
            requires(allocator_traits::template cp_constructible<T>)
        {
            size_validate(src_allocation, dst_allocation);

            allocator_traits::template construct<T>(
                allocator,
                allocation_data<T>(dst_allocation),
                allocation_cget<T>(src_allocation)
            );
        }

        constexpr void operator()(
            const callocation src_allocation,
            const allocation_type dst_allocation //
        ) const noexcept(nothrow_copy_assignable<T>)
            requires copy_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            allocation_get<T>(dst_allocation) = allocation_cget<T>(src_allocation);
        }

        constexpr void operator()(
            const allocation_type src_allocation,
            const allocation_type dst_allocation //
        ) const noexcept(nothrow_move_assignable<T>)
            requires move_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            allocation_get<T>(dst_allocation) = cpp_move(allocation_get<T>(src_allocation));
        }

        constexpr void operator()(const allocation_type allocation, allocator_type& allocator) const
            noexcept(allocator_traits::template nothrow_destructible<T>)
            requires(allocator_traits::template destructible<T>)
        {
            Expects(std::ranges::size(allocation) >= sizeof(T));
            allocator_traits::destroy(allocator, allocation_data<T>(allocation));
        }
    };

    template<allocator_req Allocator, typename T>
    class allocation_value<Allocator, T[]> // NOLINT(*-c-arrays)
    {
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocation_type = allocation_traits::allocation_type;
        using allocator_type = allocation_traits::allocator_type;
        using callocation = allocation_traits::callocation;
        using size_type = allocator_traits::size_type;

        static constexpr auto value_size = sizeof(allocation_traits::value_type);

        size_type size_;

        constexpr void size_validate(
            const callocation src_allocation,
            const callocation dst_allocation
        ) const noexcept
        {
            const auto sum_size = sizeof(T) * size_;

            Expects(std::ranges::size(src_allocation) * value_size >= sum_size);
            Expects(std::ranges::size(dst_allocation) * value_size >= sum_size);
        }

    public:
        constexpr explicit allocation_value(const size_type size) noexcept: size_(size) {}

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        bool operator==(const allocation_value&) const = default;

        constexpr void operator()(
            const callocation src_allocation,
            const allocation_type dst_allocation,
            allocator_type& allocator
        ) const noexcept(allocator_traits::template nothrow_cp_constructible<T>)
            requires(allocator_traits::template cp_constructible<T>)
        {
            size_validate(src_allocation, dst_allocation);

            for(size_type i = 0; i < size_; ++i)
                allocator_traits::template construct<T>(
                    allocator,
                    allocation_data<T>(dst_allocation) + i,
                    *std::launder(allocation_data<T>(src_allocation) + i)
                );
        }

        constexpr void operator()(
            const callocation src_allocation,
            const allocation_type dst_allocation //
        ) const noexcept(nothrow_copy_assignable<T>)
            requires copy_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            std::ranges::copy_n(
                launder_iterator{allocation_cdata<T>(src_allocation)},
                size_,
                launder_iterator{allocation_data<T>(dst_allocation)}
            );
        }

        constexpr void operator()(
            const allocation_type src_allocation,
            const allocation_type dst_allocation //
        ) const noexcept(nothrow_move_assignable<T>)
            requires move_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            std::ranges::copy_n(
                std::make_move_iterator(launder_iterator{allocation_data<T>(src_allocation)}),
                size_,
                launder_iterator{allocation_data<T>(dst_allocation)}
            );
        }

        constexpr void operator()(const allocation_type allocation, allocator_type& allocator) const
            noexcept(allocator_traits::template nothrow_destructible<T>)
            requires(allocator_traits::template destructible<T>)
        {
            Expects(allocation.size() * value_size >= sizeof(T) * size_);

            for(size_type i = 0; i < size_; ++i)
                allocator_traits::destroy(
                    allocator,
                    std::launder(allocation_data<T>(allocation) + i)
                );
        }
    };
}

namespace stdsharp::allocator_aware::details
{
    template<allocator_req Allocator, special_mem_req Req>
    struct allocation_dynamic_value_operation
    {
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocation_type = allocation_traits::allocation_type;
        using allocator_type = allocation_traits::allocator_type;
        using callocation = allocation_traits::callocation;

        using fake_type = fake_type_for<Req>;

        using dispatchers = invocables<
            dispatcher<
                get_expr_req(allocator_traits::template cp_constructible<fake_type>, allocator_traits::template nothrow_cp_constructible<fake_type>),
                void,
                const callocation&,
                const allocation_type&,
                allocator_type&>,
            dispatcher<
                get_expr_req(copy_assignable<fake_type>, nothrow_copy_assignable<fake_type>),
                void,
                const callocation&,
                const allocation_type&>,
            dispatcher<
                get_expr_req(move_assignable<fake_type>, nothrow_move_assignable<fake_type>),
                void,
                const allocation_type&,
                const allocation_type&>,
            dispatcher<
                get_expr_req(allocator_traits::template destructible<fake_type>, allocator_traits::template nothrow_destructible<fake_type>),
                void,
                const allocation_type&,
                allocator_type&>>;
    };
}

namespace stdsharp::allocator_aware
{
    template<special_mem_req Req>
    struct allocation_dynamic_type : fake_type_for<Req>
    {
    };

    template<allocator_req Allocator, special_mem_req Req>
    class allocation_value<Allocator, allocation_dynamic_type<Req>> :
        details::allocation_dynamic_value_operation<Allocator, Req>::dispatchers
    {
        using m_dispatchers =
            details::allocation_dynamic_value_operation<Allocator, Req>::dispatchers;

    public:
        allocation_value() = default;

        explicit constexpr allocation_value(const m_dispatchers dispatchers) noexcept:
            m_dispatchers(dispatchers)
        {
        }

        bool operator==(const allocation_value&) const = default;

        template<typename T, typename Op = allocation_value<T>>
            requires std::constructible_from<m_dispatchers, Op, Op, Op, Op>
        explicit constexpr allocation_value(const std::type_identity<T> /*unused*/) //
            noexcept:
            allocation_value(Op{}, Op{}, Op{}, Op{})
        {
        }

        template<typename... T, std::invocable<T...> Dispatchers = const m_dispatchers&>
        constexpr void operator()(T&&... t) const noexcept(nothrow_invocable<Dispatchers, T...>)
        {
            static_cast<Dispatchers>(*this)(cpp_forward(t)...);
        }
    };
}