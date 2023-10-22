#pragma once

#include "allocation.h"

namespace stdsharp::allocator_aware
{
    template<typename T>
    concept allocation_req = requires(
        T allocation,
        const T callocation,
        T::allocator_type alloc,
        allocator_traits<decltype(alloc)> traits,
        decltype(traits)::pointer p,
        decltype(traits)::const_pointer cp,
        decltype(traits)::size_type size
    ) {
        requires nothrow_default_initializable<T>;
        requires nothrow_constructible_from<T, const decltype(p)&, const decltype(size)&>;
        requires nothrow_copyable<T>;
        {
            callocation.begin()
        } noexcept -> std::same_as<decltype(p)>;
        {
            callocation.end()
        } noexcept -> std::same_as<decltype(p)>;
        {
            callocation.data()
        } noexcept -> std::same_as<decltype(cp)>;

        {
            callocation.cbegin()
        } noexcept -> std::same_as<decltype(cp)>;
        {
            callocation.cend()
        } noexcept -> std::same_as<decltype(cp)>;
        {
            callocation.cdata()
        } noexcept -> std::same_as<decltype(cp)>;

        {
            callocation.size()
        } noexcept -> std::same_as<decltype(size)>;

        {
            callocation.empty()
        } noexcept -> std::same_as<decltype(size)>;

        requires[]
        {
            struct local_type
            {
            };

            return requires(const T callocation) {
                {
                    callocation.template get<local_type>()
                } noexcept -> std::same_as<local_type&>;
                {
                    callocation.template cget<local_type>()
                } noexcept -> std::same_as<const local_type&>;
                {
                    callocation.template data<local_type>()
                } noexcept -> std::same_as<local_type*>;
                {
                    callocation.template cdata<local_type>()
                } noexcept -> std::same_as<const local_type*>;
            };
        }
        ();
    };

    template<typename Rng, typename Allocation>
    concept allocations_view = allocation_req<Allocation> && std::ranges::input_range<Rng> &&
        std::same_as<std::ranges::range_value_t<Rng>, Allocation>;

    template<typename Rng, typename Allocation>
    concept callocations_view = allocation_req<Allocation> && std::ranges::input_range<Rng> &&
        std::same_as<std::ranges::range_value_t<Rng>, const Allocation>;

    template<allocation_req Allocation>
    struct allocation_traits
    {
        using allocator_type = typename Allocation::allocator_type;
        using allocator_traits = allocator_traits<allocator_type>;
        using value_type = allocator_traits::value_type;
        using size_type = allocator_traits::size_type;
        using difference_type = allocator_traits::difference_type;
        using cvp = allocator_traits::const_void_pointer;
        using pointer = allocator_traits::pointer;
        using const_pointer = allocator_traits::const_pointer;
        using void_pointer = allocator_traits::void_pointer;
        using const_void_pointer = allocator_traits::const_void_pointer;

        using allocation_type = Allocation;
        using callocation = callocation<Allocation>;

        using allocation_cref = const callocation&;
        using allocator_cref = const allocator_type&;

        using default_allocations_t = std::ranges::single_view<allocation_type>;

        template<allocations_view<Allocation> Allocations = default_allocations_t>
        struct target
        {
            std::reference_wrapper<allocator_type> allocator;
            Allocations allocations;
        };

        template<typename T>
        target(auto&, T) -> target<T>;

        template<callocations_view<Allocation> Allocations = default_allocations_t>
        struct const_source
        {
            std::reference_wrapper<const allocator_type> allocator;
            Allocations allocations;
        };

        template<typename T>
        const_source(auto&, T) -> const_source<T>;

        template<allocations_view<Allocation> Allocations = default_allocations_t>
        struct source
        {
            std::reference_wrapper<allocator_type> allocator;
            Allocations allocations;

            constexpr auto to_const() const noexcept
            {
                return const_source{
                    allocator.get(), // TODO: replace with std::views::as_const
                    std::ranges::subrange{
                        std::ranges::cbegin(allocations),
                        std::ranges::cend(allocations)
                    }
                };
            }
        };

        template<typename T>
        source(auto&, T) -> source<T>;

        template<allocations_view<Allocation> Allocations = default_allocations_t>
        struct construction_result
        {
            allocator_type allocator;
            Allocations allocation;
        };

        template<typename T>
        construction_result(auto, T) -> construction_result<T>;

        static constexpr allocation_type
            allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr)
        {
            return {allocator_traits::allocate(alloc, size, hint), size};
        }

        static constexpr allocation_type
            try_allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr) //
            noexcept
        {
            return {allocator_traits::try_allocate(alloc, size, hint), size};
        }

        template<typename T>
        static constexpr void deallocate(const target<T> dst) noexcept
        {
            for(const auto allocation : dst.allocations)
            {
                allocator_traits::deallocate(dst.allocator, allocation.data(), allocation.size());
                allocation = {};
            }
        }
    };
}