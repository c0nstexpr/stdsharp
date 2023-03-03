#pragma once

#include "allocator_traits.h"
#include "../exception/exception.h"
#include "pointer_traits.h"

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

        using first_ptr = typename first_traits::pointer;
        using second_ptr = typename second_traits::pointer;

        using first_cvp = typename first_traits::const_void_pointer;
        using second_cvp = typename second_traits::const_void_pointer;

        using first_ptr_traits = pointer_traits<first_ptr>;
        using second_ptr_traits = pointer_traits<second_ptr>;

        using first_cvp_traits = pointer_traits<first_cvp>;
        using second_cvp_traits = pointer_traits<second_cvp>;

    public:
        using value_type = typename FirstAlloc::value_type;

        using propagate_on_container_copy_assignment = ::std::disjunction<
            typename first_traits::propagate_on_container_copy_assignment,
            typename second_traits::propagate_on_container_copy_assignment>;
        using propagate_on_container_move_assignment = ::std::disjunction<
            typename first_traits::propagate_on_container_move_assignment,
            typename second_traits::propagate_on_container_move_assignment>;
        using propagate_on_container_swap = ::std::disjunction<
            typename first_traits::propagate_on_container_swap,
            typename second_traits::propagate_on_container_swap>;

        using is_always_equal = ::std::conjunction<
            typename first_traits::is_always_equal,
            typename second_traits::is_always_equal>;

        composed_allocator() = default;

        template<typename... Args>
        constexpr explicit composed_allocator(Args&&... args):
            alloc_pair_(::std::forward<Args>(args)...)
        {
        }

        constexpr auto allocate(const ::std::size_t n, const void* const hint = nullptr)
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

        constexpr auto
            try_allocate(const ::std::size_t n, const void* const hint = nullptr) noexcept
        {
            const auto ptr = first_traits::try_allocate(
                alloc_pair_.first,
                n,
                first_cvp_traits::to_pointer(hint)
            );
            return ptr == nullptr ? //
                second_traits::try_allocate(
                    alloc_pair_.second,
                    n,
                    second_cvp_traits::to_pointer(hint)
                ) :
                ptr;
        }

        constexpr void
            deallocate(typename allocator_traits<FirstAlloc>::pointer ptr, const ::std::size_t n)
        {
            auto& [first, second] = alloc_pair_;
            if(first.contains(ptr)) first.deallocate(ptr, n);
            else second.deallocate(ptr, n);
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

                    result = {.ptr = first_ptr_traits::to_address(res.ptr), .count = res.count};
                },
                [&result, this, n]
                {
                    const auto& res = second_traits::allocate_at_least(alloc_pair_.second, n);

                    result = {
                        .ptr = second_ptr_traits::to_address(res.ptr),
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
            auto& [first, second] = alloc_pair_;
            if( //
                first_traits::contains(
                    first,
                    first_ptr_traits::pointer_to(
                        static_cast<value_type*>(static_cast<void*>(ptr))
                    )
                )
            )
                first_traits::construct(first, ptr, ::std::forward<Args>(args)...);
            else second_traits::construct(second, ptr, ::std::forward<Args>(args)...);
        }

        constexpr bool contains(const value_type* const ptr) const noexcept
            requires details::allocator_contains<SecondAlloc>
        {
            return alloc_pair_.first.contains(first_cvp_traits::to_pointer(ptr)) ?
                alloc_pair_.second.contains(second_cvp_traits::to_pointer(ptr)) :
                false;
        }
    };

    template<typename T, typename U>
    composed_allocator(T&&, U&&) -> composed_allocator<::std::decay_t<T>, ::std::decay_t<U>>;
}