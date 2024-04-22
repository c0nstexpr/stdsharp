#pragma once

#include "allocation_traits.h"
#include "launder_iterator.h"

#include <algorithm>

namespace stdsharp::details
{
    template<typename Allocator, typename T>
    struct allocation_value
    {
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocator_type = allocation_traits::allocator_type;
        using size_type = allocator_traits::size_type;

        using ctor = allocator_traits::constructor;
        using dtor = allocator_traits::destructor;

        template<typename>
        struct forward_value;

        template<allocation<Allocator> Allocation>
        struct forward_value<Allocation>
        {
            using type = T&&;
        };

        template<callocation<Allocator> Allocation>
        struct forward_value<Allocation>
        {
            using type = const T&;
        };

        template<typename Allocation>
        using forward_value_t = forward_value<Allocation>::type;

        static constexpr void size_validate(const auto& t, const auto&... allocations) noexcept
        {
            ( //
                Expects(
                    allocations.size() * sizeof(typename allocation_traits::value_type) >=
                    t.value_size()
                ),
                ...
            );
        }

        [[nodiscard]] constexpr auto data(this const auto& t, const auto& allocation) noexcept
        {
            size_validate(t, allocation);
            return allocation_traits::template data<T>(allocation);
        }

        [[nodiscard]] constexpr decltype(auto) get(this const auto& t, const auto& allocation) //
            noexcept
        {
            size_validate(t, allocation);
            return allocation_traits::template get<T>(allocation);
        }

        static constexpr struct default_construct_t
        {
        } default_construct{};
    };
}

namespace stdsharp
{
    template<allocator_req Allocator, typename T = Allocator::value_type>
    struct allocation_value : private details::allocation_value<Allocator, T>
    {
    private:
        using m_base = details::allocation_value<Allocator, T>;

        friend m_base;

    public:
        using typename m_base::allocator_type;
        using m_base::data;
        using m_base::get;

        template<typename Allocation>
        using forward_value_t = m_base::template forward_value_t<Allocation>;

        static constexpr auto always_equal = true;

        [[nodiscard]] static constexpr auto value_size() noexcept { return sizeof(T); }

        [[nodiscard]] bool operator==(const allocation_value&) const = default;

    private:
        using typename m_base::ctor;
        using typename m_base::dtor;

        template<typename Allocation>
        [[nodiscard]] constexpr decltype(auto) forward_value(const Allocation& src) const noexcept
        {
            return static_cast<forward_value_t<Allocation>>(get(src));
        }

    public:
        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation,
            const m_base::default_construct_t /*unused*/
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*>)
            requires std::invocable<ctor, allocator_type&, T*>
        {
            ctor{}(allocator, data(allocation));
        }

        template<typename SrcAllocation, typename Value = forward_value_t<SrcAllocation>>
            requires std::invocable<ctor, allocator_type&, T*, Value>
        constexpr void operator()(
            allocator_type& allocator,
            const SrcAllocation& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*, Value>)
        {
            ctor{}(allocator, data(dst_allocation), forward_value(src_allocation));
        }

        template<typename Allocation, typename Value = forward_value_t<Allocation>>
        constexpr void operator()(
            const Allocation& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(nothrow_assignable_from<T&, Value>)
            requires std::assignable_from<T&, Value>
        {
            get(dst_allocation) = forward_value(src_allocation);
        }

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation
        ) const noexcept
        {
            dtor{}(allocator, data(allocation));
        }
    };

    template<allocator_req Allocator, typename T>
    struct allocation_value<Allocator, T[]> :// NOLINT(*-c-arrays)
        private details::allocation_value<Allocator, T>
    {
    private:
        using m_base = details::allocation_value<Allocator, T>;

        friend m_base;

    public:
        using typename m_base::allocator_type;

    private:
        using typename m_base::ctor;
        using typename m_base::dtor;
        using size_t = std::size_t;

        size_t size_;

    public:
        [[nodiscard]] constexpr auto data(const auto& allocation) const noexcept
        {
            return launder_iterator{m_base::data(allocation)};
        }

    private:
        template<typename Allocation>
        using forward_value_t = m_base::template forward_value_t<Allocation>;

        [[nodiscard]] constexpr auto forward_data(const callocation<Allocator> auto& allocation) //
            const noexcept
        {
            return data(allocation);
        }

        [[nodiscard]] constexpr auto forward_data(const allocation<Allocator> auto& allocation) //
            const noexcept
        {
            return std::move_iterator{data(allocation)};
        }

    public:
        [[nodiscard]] constexpr auto get(const auto& allocation) const noexcept
        {
            return std::views::counted(data(allocation), size());
        }

        constexpr explicit allocation_value(const m_base::size_type size) noexcept: size_(size) {}

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        [[nodiscard]] constexpr auto value_size() const noexcept { return size() * sizeof(T); }

        [[nodiscard]] bool operator==(const allocation_value&) const = default;

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation,
            const m_base::default_construct_t /*unused*/
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*>)
            requires std::invocable<ctor, allocator_type&, T*>
        {
            auto begin = m_base::data(allocation);
            for(const auto end = begin + size(); begin < end; ++begin) ctor{}(allocator, begin);
        }

        template<typename Allocation, typename Value = forward_value_t<Allocation>>
        constexpr void operator()(
            allocator_type& allocator,
            const Allocation& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*, Value>)
            requires std::invocable<ctor, allocator_type&, T*, Value>
        {
            const auto& dst_begin = m_base::data(dst_allocation);
            const auto& src_begin = forward_data(src_allocation);
            const auto count = size();

            for(size_t i = 0; i < count; ++i) ctor{}(allocator, dst_begin + i, src_begin[i]);
        }

        template<typename Allocation, typename Value = forward_value_t<Allocation>>
        constexpr void operator()(
            const Allocation& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(nothrow_assignable_from<T&, Value>)
            requires std::assignable_from<T&, Value>
        {
            std::ranges::copy_n(forward_data(dst_allocation), size(), data(src_allocation));
        }

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation
        ) const noexcept
        {
            auto&& begin = m_base::data(allocation);
            for(const auto& end = begin + size(); begin < end; ++begin)
                dtor{}(allocator, std::launder(begin));
        }
    };
}