#pragma once

#include "allocator_traits.h"
#include "../exception/exception.h"

namespace stdsharp
{
    namespace details
    {
        template<typename T>
        concept allocator_contains = requires(
            const T alloc,
            const typename allocator_traits<T>::pointer ptr
        ) // clang-format off
        {
            { alloc.contains(ptr) } -> ::std::convertible_to<bool>; // clang-format on
            requires noexcept(alloc.contains(ptr));
        };
    }

    template<details::allocator_contains FirstAlloc, allocator_req SecondAlloc>
        requires ::std::same_as<typename FirstAlloc::value_type, typename SecondAlloc::value_type>
    class composed_allocator
    {
        ::std::pair<FirstAlloc, SecondAlloc> alloc_pair_;

        using first_traits = allocator_traits<FirstAlloc>;
        using second_traits = allocator_traits<SecondAlloc>;

        using first_pointer = typename first_traits::pointer;
        using second_pointer = typename second_traits::pointer;

        using first_cvp = typename first_traits::const_void_pointer;
        using second_cvp = typename second_traits::const_void_pointer;

        using first_pointer_traits = ::std::pointer_traits<first_pointer>;
        using second_pointer_traits = ::std::pointer_traits<second_pointer>;

        using first_cvp_traits = ::std::pointer_traits<first_cvp>;
        using second_cvp_traits = ::std::pointer_traits<second_cvp>;

    public:
        using value_type = typename FirstAlloc::value_type;

        template<typename... Args>
        constexpr explicit composed_allocator(Args&&... args):
            alloc_pair_(::std::forward<Args>(args)...)
        {
        }

        constexpr auto allocate(const ::std::size_t n, const void* const hint)
        {
            const auto ptr = first_traits::try_allocate(
                alloc_pair_.first,
                n,
                first_cvp_traits::to_pointer(hint)
            );
            return ptr == nullptr ? //
                second_traits::allocate(
                    alloc_pair_.second,
                    n,
                    second_cvp_traits::to_pointer(hint)
                ) :
                ptr;
        }

        constexpr void
            deallocate(const ::std::size_t n, typename allocator_traits<FirstAlloc>::pointer ptr)
        {
            auto& [first, second] = alloc_pair_;
            if(first.contains(ptr)) first.deallocate(n, ptr);
            else second.deallocate(n, ptr);
        }

        constexpr auto max_size() const noexcept
        {
            return ::std::min(
                first_traits::max_size(alloc_pair_.first),
                second_traits::max_size(alloc_pair_.second)
            );
        }

        constexpr composed_allocator select_on_container_copy_construction() const
        {
            return {
                first_traits::select_on_container_copy_construction(alloc_pair_.first),
                second_traits::select_on_container_copy_construction(alloc_pair_.second) //
            };
        }

        constexpr auto allocate_at_least(const ::std::size_t n)
        {
            struct
            {
                value_type* ptr;
                ::std::size_t count;
            } result;

            aggregate_try(
                [&result, this, n]
                {
                    const auto& res = first_traits::allocate_at_least(alloc_pair_.first, n);

                    result = {.ptr = first_pointer_traits::to_address(res.ptr), .count = res.count};
                },
                [&result, this, n]
                {
                    const auto& res = second_traits::allocate_at_least(alloc_pair_.second, n);

                    result = {
                        .ptr = second_pointer_traits::to_address(res.ptr),
                        .count = res.count //
                    };
                }
            );

            return result;
        }

        template<typename T, typename... Args>
        constexpr void construct(T* const ptr, Args&&... args) //
            noexcept(nothrow_constructible_from<value_type, Args...>)
        {
            const auto value_t_p = static_cast<value_type*>(static_cast<void*>(ptr));
            auto& [first, second] = alloc_pair_;
            if(first_traits::contains(first, first_pointer_traits::pointer_to(value_t_p)))
                first_traits::construct(first, value_t_p, ::std::forward<Args>(args)...);
            else second_traits::construct(second, value_t_p, ::std::forward<Args>(args)...);
        }

        constexpr bool contains(const value_type* const ptr) const noexcept
            requires details::allocator_contains<SecondAlloc>
        {
            return alloc_pair_.first.contains(first_cvp_traits::to_pointer(ptr)) ||
                alloc_pair_.second.contains(second_cvp_traits::to_pointer(ptr));
        }
    };
}