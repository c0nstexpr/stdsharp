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

        static constexpr void size_validate(const auto& src, const auto& dst)
        {
            Expects(allocation_traits::size(src) * element_size >= value_size());
            Expects(allocation_traits::size(dst) * element_size >= value_size());
        }

        using ctor = allocator_traits::constructor;
        using ctor_uses_alloc = allocator_traits::constructor_uses_allocator;
        using dtor = allocator_traits::destructor;

        static constexpr auto data = allocation_traits::template data<T>;

        using data_fn = decltype(data);

        static constexpr auto get = allocation_traits::template get<T>;

        using get_fn = decltype(get);

        template<typename Allocation>
        using get_t = std::invoke_result_t<get_fn, const Allocation&>;

    public:
        static constexpr auto always_equal = true;

        [[nodiscard]] static constexpr auto value_size() noexcept { return sizeof(T); }

        bool operator==(const allocation_value&) const = default;

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation,
            const empty_t /*unused*/
        ) const noexcept(nothrow_invocable<ctor_uses_alloc, allocator_type&, T*>)
            requires std::invocable<ctor_uses_alloc, allocator_type&, T*>
        {
            Expects(allocation_traits::size(allocation) * element_size >= value_size());
            ctor_uses_alloc{}(allocator, data(allocation));
        }

        template<typename Src>
        constexpr void operator()(
            allocator_type& allocator,
            const Src& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*, get_t<Src>>)
            requires(callocation<Src, Allocator> || allocation<Src, Allocator>) &&
            std::invocable<ctor, allocator_type&, T*, get_t<Src>>
        {
            size_validate(src_allocation, dst_allocation);
            ctor{}(allocator, data(dst_allocation), get(src_allocation));
        }

        constexpr void operator()(
            const callocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation //
        ) const noexcept(nothrow_copy_assignable<T>)
            requires copy_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            get(dst_allocation) = get(src_allocation);
        }

        constexpr void operator()(
            const allocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation //
        ) const noexcept(nothrow_move_assignable<T>)
            requires move_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            get(dst_allocation) = cpp_move(get(src_allocation));
        }

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation //
        ) const noexcept(nothrow_invocable<dtor, allocator_type&, T*>)
            requires std::invocable<dtor, allocator_type&, T*>
        {
            Expects(allocation_traits::size(allocation) * element_size >= value_size());
            dtor{}(allocator, data(allocation));
        }
    };

    template<allocator_req Allocator, typename T>
    struct allocation_value<Allocator, T[]> // NOLINT(*-c-arrays)
    {
    public:
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocator_type = allocation_traits::allocator_type;
        using size_type = allocator_traits::size_type;

    private:
        using ctor = allocator_traits::constructor;
        using ctor_uses_alloc = allocator_traits::constructor_uses_allocator;
        using dtor = allocator_traits::destructor;

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
            const allocation<Allocator> auto& allocation,
            const empty_t /*unused*/
        ) const noexcept(nothrow_invocable<ctor_uses_alloc, allocator_type&, T*>)
            requires std::invocable<ctor_uses_alloc, allocator_type&, T*>
        {
            Expects(allocation.size() * element_size >= sizeof(T) * size_);

            for(size_type i = 0; i < size_; ++i)
                ctor_uses_alloc{}( //
                    allocator,
                    std::launder(allocation_traits::template data<T>(allocation) + i)
                );
        }

        constexpr void operator()(
            allocator_type& allocator,
            const callocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*, const T&>)
            requires std::invocable<ctor, allocator_type&, T*, const T&>
        {
            size_validate(src_allocation, dst_allocation);
            for(std::size_t i = 0; i < size_; ++i)
                ctor{}( //
                    allocator,
                    allocation_traits::template data<T>(dst_allocation) + i,
                    *std::launder(allocation_traits::template data<T>(src_allocation) + i)
                );
        }

        constexpr void operator()(
            allocator_type& allocator,
            const callocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*, T>)
            requires std::invocable<ctor, allocator_type&, T*, T>
        {
            size_validate(src_allocation, dst_allocation);
            for(std::size_t i = 0; i < size_; ++i)
                ctor{}( //
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

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation //
        ) const noexcept(nothrow_invocable<dtor, allocator_type&, T*>)
            requires std::invocable<dtor, allocator_type&, T*>
        {
            Expects(allocation.size() * element_size >= sizeof(T) * size_);

            for(size_type i = 0; i < size_; ++i)
                dtor{}( //
                    allocator,
                    std::launder(allocation_traits::template data<T>(allocation) + i)
                );
        }
    };
}