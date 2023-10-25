#pragma once

#include "allocation_traits.h"
#include "../../type_traits/special_member.h"
#include "../../utility/dispatcher.h"
#include "../launder_iterator.h"

namespace stdsharp::allocator_aware
{
    template<allocator_req Allocator, typename T = Allocator::value_type>
    struct allocation_value_operation
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
            Expects(dst_allocation.size() >= sizeof(T));
            Expects(src_allocation.size() >= sizeof(T));
        }

    public:
        static constexpr auto always_equal = true;

        bool operator==(const allocation_value_operation&) const = default;

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
                dst_allocation.template data<T>(),
                src_allocation.template cget<T>()
            );
        }

        constexpr void operator()(
            const callocation src_allocation,
            const allocation_type dst_allocation //
        ) const noexcept(nothrow_copy_assignable<T>)
            requires copy_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            dst_allocation.template get<T>() = src_allocation.template cget<T>();
        }

        constexpr void operator()(
            const allocation_type src_allocation,
            const allocation_type dst_allocation //
        ) const noexcept(nothrow_move_assignable<T>)
            requires move_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            dst_allocation.template get<T>() = cpp_move(src_allocation.template get<T>());
        }

        constexpr void operator()(const allocation_type allocation, allocator_type& allocator) const
            noexcept(allocator_traits::template nothrow_destructible<T>)
            requires(allocator_traits::template destructible<T>)
        {
            Expects(allocation.size() >= sizeof(T));
            allocator_traits::destroy(allocator, allocation.template data<T>());
        }
    };

    template<allocator_req Allocator, typename T>
    class allocation_value_operation<Allocator, T[]> // NOLINT(*-c-arrays)
    {
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocation_type = allocation_traits::allocation_type;
        using allocator_type = allocation_traits::allocator_type;
        using callocation = allocation_traits::callocation;
        using value_type = allocation_traits::value_type;

        allocator_traits::size_type size_;

        constexpr void size_validate(
            const callocation src_allocation,
            const callocation dst_allocation
        ) const noexcept
        {
            const auto sum_size = sizeof(T) * size_;
            Expects(dst_allocation.size() * sizeof(value_type) >= sum_size);
            Expects(src_allocation.size() * sizeof(value_type) >= sum_size);
        }

    public:
        constexpr explicit allocation_value_operation(const std::size_t size) noexcept: size_(size)
        {
        }

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        bool operator==(const allocation_value_operation&) const = default;

        constexpr void operator()(
            const callocation src_allocation,
            const allocation_type dst_allocation,
            allocator_type& allocator
        ) const noexcept(allocator_traits::template nothrow_cp_constructible<T>)
            requires(allocator_traits::template cp_constructible<T>)
        {
            size_validate(src_allocation, dst_allocation);

            for(std::size_t i = 0; i < size_; ++i)
                allocator_traits::template construct<T>(
                    allocator,
                    dst_allocation.template data<T>() + i,
                    std::launder(src_allocation.template data<T>() + i)
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
                launder_iterator{src_allocation.template cdata<T>()},
                size_,
                launder_iterator{dst_allocation.template data<T>()}
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
                std::make_move_iterator(launder_iterator{src_allocation.template data<T>()}),
                size_,
                launder_iterator{dst_allocation.template data<T>()}
            );
        }

        constexpr void operator()(const allocation_type allocation, allocator_type& allocator) const
            noexcept(allocator_traits::template nothrow_destructible<T>)
            requires(allocator_traits::template destructible<T>)
        {
            Expects(allocation.size() * sizeof(value_type) >= sizeof(T) * size_);

            for(std::size_t i = 0; i < size_; ++i) allocator_traits::destroy(allocator, std::launder(allocation.template data<T>() + i));
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
    class allocation_value_operation<Allocator, allocation_dynamic_type<Req>> :
        details::allocation_dynamic_value_operation<Allocator, Req>::dispatchers
    {
        using m_dispatchers =
            details::allocation_dynamic_value_operation<Allocator, Req>::dispatchers;

    public:
        allocation_value_operation() = default;

        explicit constexpr allocation_value_operation(const m_dispatchers dispatchers) noexcept:
            m_dispatchers(dispatchers)
        {
        }

        bool operator==(const allocation_value_operation&) const = default;

        template<typename T, typename Op = allocation_value_operation<T>>
            requires std::constructible_from<m_dispatchers, Op, Op, Op, Op>
        explicit constexpr allocation_value_operation(const std::type_identity<T> /*unused*/) //
            noexcept:
            allocation_value_operation(Op{}, Op{}, Op{}, Op{})
        {
        }

        template<typename... T, std::invocable<T...> Dispatchers = const m_dispatchers&>
        constexpr void operator()(T&&... t) const noexcept(nothrow_invocable<Dispatchers, T...>)
        {
            static_cast<Dispatchers>(*this)(cpp_forward(t)...);
        }
    };
}