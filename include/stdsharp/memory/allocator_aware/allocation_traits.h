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

    template<allocation_req Allocation>
    struct allocation_traits
    {
        using allocator_type = typename Allocation::allocator_type;
        using allocator_traits = allocator_traits<allocator_type>;
        using value_type = allocator_traits::value_type;
        using size_type = allocator_traits::size_type;
        using cvp = allocator_traits::const_void_pointer;
        using pointer = allocator_traits::pointer;
        using const_pointer = allocator_traits::const_pointer;

        using allocation_type = Allocation;
        using callocation = callocation<Allocation>;

        using allocation_cref = const callocation&;
        using allocator_cref = const allocator_type&;

        struct destination
        {
            std::reference_wrapper<allocator_type> allocator;
            std::reference_wrapper<allocation_type> allocation;
        };

        struct const_source
        {
            std::reference_wrapper<const allocator_type> allocator;
            std::reference_wrapper<const callocation> allocation;
        };

        struct source
        {
            std::reference_wrapper<allocator_type> allocator;
            std::reference_wrapper<allocation_type> allocation;

            constexpr operator const_source() const noexcept
            {
                return {allocator.get(), allocation.get()};
            }
        };

        struct construction_result
        {
            allocator_type allocator;
            allocation_type allocation;
        };

        static constexpr allocation_type
            allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr)
        {
            return {allocator_traits::allocate(alloc, size, hint), size};
        }

        static constexpr allocation_type try_allocate(
            allocator_type& alloc,
            const size_type size,
            const cvp hint = nullptr
        ) noexcept
        {
            return {allocator_traits::try_allocate(alloc, size, hint), size};
        }

        static constexpr void deallocate(const destination dst) noexcept
        {
            auto& allocation = dst.allocation.get();
            allocator_traits::deallocate(dst.allocator, allocation.data(), allocation.size());
            allocation = {};
        }
    };
}