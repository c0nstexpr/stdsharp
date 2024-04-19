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

        static constexpr auto data = allocation_traits::template data<T>;
        static constexpr auto get = allocation_traits::template get<T>;

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

        constexpr void size_validate(this const auto& t, const auto&... allocations) noexcept
        {
            ( //
                Expects(
                    allocations.size() * sizeof(typename allocation_traits::value_type) >=
                    t.value_size()
                ),
                ...
            );
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
        using m_base = details::allocation_value<Allocator, T>;

    public:
        using typename m_base::allocation_traits;
        using typename m_base::allocator_traits;
        using typename m_base::allocator_type;
        using typename m_base::size_type;
        using typename m_base::default_construct_t;
        using m_base::default_construct;
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
        using m_base::size_validate;

        template<typename Allocation>
        [[nodiscard]] constexpr decltype(auto) forward_value(const Allocation& src_allocation
        ) const noexcept
        {
            size_validate(src_allocation);
            return static_cast<forward_value_t<Allocation>>(get(src_allocation));
        }

    public:
        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation,
            const default_construct_t /*unused*/
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*>)
            requires std::invocable<ctor, allocator_type&, T*>
        {
            size_validate(allocation);
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
            size_validate(dst_allocation);
            ctor{}(allocator, data(dst_allocation), forward_value(src_allocation));
        }

        template<typename Allocation, typename Value = forward_value_t<Allocation>>
        constexpr void operator()(
            const Allocation& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(nothrow_assignable_from<T&, Value>)
            requires std::assignable_from<T&, Value>
        {
            size_validate(dst_allocation);
            get(dst_allocation) = forward_value(src_allocation);
        }

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation
        ) const noexcept
        {
            size_validate(allocation);
            dtor{}(allocator, data(allocation));
        }
    };

    template<allocator_req Allocator, typename T>
    struct allocation_value<Allocator, T[]> :// NOLINT(*-c-arrays)
        private details::allocation_value<Allocator, T>
    {
        using m_base = details::allocation_value<Allocator, T>;

    public:
        using typename m_base::allocation_traits;
        using typename m_base::allocator_traits;
        using typename m_base::allocator_type;
        using typename m_base::size_type;
        using typename m_base::default_construct_t;
        using m_base::default_construct;
        using m_base::data;
        using m_base::get;

    private:
        using typename m_base::ctor;
        using typename m_base::dtor;
        using size_t = std::size_t;
        using m_base::size_validate;

        size_t size_;

        [[nodiscard]] constexpr auto launder_begin(const auto& allocation) const noexcept
        {
            size_validate(allocation);
            return launder_iterator{data(allocation)};
        }

        [[nodiscard]] constexpr auto
            forward_launder_begin(const callocation<Allocator> auto& allocation) const noexcept
        {
            return launder_begin(allocation);
        }

        [[nodiscard]] constexpr auto
            forward_launder_begin(const allocation<Allocator> auto& allocation) const noexcept
        {
            return std::move_iterator{launder_begin(allocation)};
        }

        template<typename Allocation>
        using forward_value_t = m_base::template forward_value_t<Allocation>;

        [[nodiscard]] constexpr auto forward_rng(const auto& allocation) const noexcept
        {
            return std::views::counted(forward_launder_begin(allocation), size());
        }

    public:
        constexpr explicit allocation_value(const size_type size) noexcept: size_(size) {}

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        [[nodiscard]] constexpr auto value_size() const noexcept { return size() * sizeof(T); }

        [[nodiscard]] bool operator==(const allocation_value&) const = default;

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation,
            const default_construct_t /*unused*/
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*>)
            requires std::invocable<ctor, allocator_type&, T*>
        {
            size_validate(allocation);
            auto begin = data(allocation);
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
            size_validate(dst_allocation);

            const auto& dst_begin = data(dst_allocation);
            const auto& src_begin = forward_launder_begin(src_allocation);
            const auto count = size();

            for(size_t i = 0; i < count; ++i) ctor{}(allocator, dst_begin[i], src_begin[i]);
        }

        template<typename Allocation, typename Value = forward_value_t<Allocation>>
        constexpr void operator()(
            const Allocation& src_allocation,
            const allocation<Allocator> auto& dst_allocation
        ) const noexcept(nothrow_assignable_from<T, Value>)
            requires std::assignable_from<T, Value>
        {
            std::ranges::copy_n(
                launder_begin(src_allocation),
                size(),
                forward_launder_begin(dst_allocation)
            );
        }

        constexpr void operator()(
            allocator_type& allocator,
            const allocation<Allocator> auto& allocation
        ) const noexcept
        {
            auto&& begin = launder_begin(allocation);
            for(const auto& end = begin + size(); begin < end; ++begin) dtor{}(allocator, begin);
        }
    };
}