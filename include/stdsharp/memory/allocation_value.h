#pragma once

#include "../algorithm/algorithm.h"
#include "allocation_traits.h"
#include "launder_iterator.h"

namespace stdsharp
{
    template<allocator_req Allocator, typename T = Allocator::value_type>
    struct allocation_value
    {
        using impl_t = allocation_value<Allocator, T[]>; // NOLINT(*-c-arrays)

    public:
        static constexpr auto always_equal = true;

        [[nodiscard]] static constexpr auto value_size() noexcept { return sizeof(T); }

        [[nodiscard]] bool operator==(const allocation_value&) const = default;

        template<typename... Args>
        constexpr void operator()(Args&&... args) const noexcept(nothrow_invocable<impl_t, Args...>)
            requires std::invocable<impl_t, Args...>
        {
            impl_t{1}(cpp_forward(args)...);
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
        using dtor = allocator_traits::destructor;

        static constexpr auto data = allocation_traits::template data<T>;

        using data_fn = decltype(data);

        std::size_t size_;

        constexpr void size_validate(const auto& allocation) const noexcept
        {
            Expects(
                allocation.size() * sizeof(typename allocation_traits::value_type) >= value_size()
            );
        }

        constexpr void
            size_validate(const auto& src_allocation, const auto& dst_allocation) const noexcept
        {
            size_validate(src_allocation);
            size_validate(dst_allocation);
        }

        [[nodiscard]] constexpr auto launder_begin(const auto& allocation) const noexcept
        {
            return launder_iterator{data(allocation)};
        }

        [[nodiscard]] constexpr auto launder_rng(const auto& allocation) const noexcept
        {
            return std::views::iota(launder_begin(allocation)) | std::views::take(size());
        }

        [[nodiscard]] constexpr auto value_rng(const auto& allocation) const noexcept
        {
            return std::views::counted(launder_begin(allocation), size());
        }

    public:
        constexpr explicit allocation_value(const size_type size) noexcept: size_(size) {}

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        [[nodiscard]] constexpr auto value_size() const noexcept { return size() * sizeof(T); }

        [[nodiscard]] bool operator==(const allocation_value&) const = default;

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation,
            const empty_t /*unused*/
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*>)
            requires std::invocable<ctor, allocator_type&, T*>
        {
            size_validate(allocation);

            for(const auto iter : launder_rng(allocation)) ctor{}(allocator, iter.data());
        }

        constexpr void operator()(
            allocator_type& allocator,
            const callocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*, const T&>)
            requires std::invocable<ctor, allocator_type&, T*, const T&>
        {
            size_validate(src_allocation, dst_allocation);

            for(auto dst_begin = launder_iterator{data(dst_allocation)};
                const auto& value : value_rng(src_allocation))
                ctor{}(allocator, dst_begin++.data(), value);
        }

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*, T>)
            requires std::invocable<ctor, allocator_type&, T*, T>
        {
            size_validate(src_allocation, dst_allocation);

            for(auto dst_begin = launder_iterator{data(dst_allocation)};
                auto& value : value_rng(src_allocation))
                ctor{}(allocator, dst_begin++.data(), cpp_move(value));
        }

        constexpr void operator()(
            const callocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation //
        ) const noexcept(nothrow_copy_assignable<T>)
            requires copy_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            std::ranges::copy_n(
                launder_iterator{data(src_allocation)},
                size(),
                launder_iterator{data(dst_allocation)}
            );
        }

        constexpr void operator()(
            const allocation<Allocator> auto& src_allocation,
            const allocation<Allocator> auto& dst_allocation //
        ) const noexcept(nothrow_move_assignable<T>)
            requires move_assignable<T>
        {
            size_validate(src_allocation, dst_allocation);

            move_n(
                launder_iterator{data(src_allocation)},
                size(),
                launder_iterator{data(dst_allocation)}
            );
        }

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation
        ) const noexcept
        {
            size_validate(allocation);

            for(const auto iter : launder_rng(allocation)) dtor{}(allocator, iter.data());
        }
    };
}