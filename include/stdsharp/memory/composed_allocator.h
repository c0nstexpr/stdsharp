#pragma once

#include "../exception/exception.h"
#include "allocator_traits.h"

namespace stdsharp
{
    template<typename T>
    concept allocator_contains = requires(const T& alloc, const allocator_cvp<T> cvp) {
        { alloc.contains(cvp) } noexcept -> nothrow_boolean_testable;
    };

    template<allocator_contains FirstAlloc, allocator_req SecondAlloc>
        requires requires(FirstAlloc::value_type value, const void* ptr) {
            requires std::same_as<decltype(value), typename SecondAlloc::value_type>;
            {
                pointer_traits<
                    typename allocator_traits<FirstAlloc>::const_void_pointer>::to_pointer(ptr)
            } noexcept;

            {
                pointer_traits<
                    typename allocator_traits<SecondAlloc>::const_void_pointer>::to_pointer(ptr)
            } noexcept;
        }
    class composed_allocator
    {
    public:
        using first_allocator_type = FirstAlloc;
        using second_allocator_type = SecondAlloc;

    private:
        std::pair<first_allocator_type, second_allocator_type> alloc_pair_;

        using first_traits = allocator_traits<first_allocator_type>;
        using second_traits = allocator_traits<second_allocator_type>;

        using first_ptr = first_traits::pointer;
        using second_ptr = second_traits::pointer;

        using first_cvp = first_traits::const_void_pointer;
        using second_cvp = second_traits::const_void_pointer;

        using first_ptr_traits = pointer_traits<first_ptr>;
        using second_ptr_traits = pointer_traits<second_ptr>;

        using first_cvp_traits = pointer_traits<first_cvp>;
        using second_cvp_traits = pointer_traits<second_cvp>;

    public:
        using value_type = FirstAlloc::value_type;

        using propagate_on_container_copy_assignment = std::disjunction<
            typename first_traits::propagate_on_container_copy_assignment,
            typename second_traits::propagate_on_container_copy_assignment>;
        using propagate_on_container_move_assignment = std::disjunction<
            typename first_traits::propagate_on_container_move_assignment,
            typename second_traits::propagate_on_container_move_assignment>;
        using propagate_on_container_swap = std::disjunction<
            typename first_traits::propagate_on_container_swap,
            typename second_traits::propagate_on_container_swap>;

        using is_always_equal = std::conjunction<
            typename first_traits::is_always_equal,
            typename second_traits::is_always_equal>;

        template<typename T>
        struct rebind
        {
            using other = composed_allocator<
                typename first_traits::template rebind_alloc<T>,
                typename second_traits::template rebind_alloc<T>>;
        };

        composed_allocator() = default;

        template<typename... Args, std::constructible_from<Args...> Pair = decltype(alloc_pair_)>
        constexpr explicit(sizeof...(Args) == 1) composed_allocator(Args&&... args)
            noexcept(nothrow_constructible_from<Pair, Args...>):
            alloc_pair_(cpp_forward(args)...)
        {
        }

        [[nodiscard]] constexpr auto allocate(const std::size_t n, const void* const hint = nullptr)
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

        [[nodiscard]] constexpr auto
            try_allocate(const std::size_t n, const void* const hint = nullptr) noexcept
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

        constexpr void deallocate(value_type* const ptr, const std::size_t n) noexcept
        {
            auto& [first, second] = alloc_pair_;
            if(first.contains(first_cvp_traits::to_pointer(static_cast<const void*>(ptr))))
                first.deallocate(ptr, n);
            else second.deallocate(ptr, n);
        }

        [[nodiscard]] constexpr auto max_size() const noexcept
        {
            return std::min(
                first_traits::max_size(alloc_pair_.first),
                second_traits::max_size(alloc_pair_.second)
            );
        }

        [[nodiscard]] constexpr composed_allocator select_on_container_copy_construction() const
        {
            return {
                first_traits::select_on_container_copy_construction(alloc_pair_.first),
                second_traits::select_on_container_copy_construction(alloc_pair_.second)
            };
        }

        [[nodiscard]] constexpr auto allocate_at_least(const std::size_t n)
        {
            struct
            {
                value_type* ptr;
                std::size_t count;
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

                    result = {.ptr = second_ptr_traits::to_address(res.ptr), .count = res.count};
                }
            );

            return result;
        }

        template<
            typename T,
            typename... Args,
            typename FirstCtor = first_traits::constructor,
            typename SecondCtor = second_traits::constructor>
            requires std::invocable<FirstCtor, first_allocator_type&, T*, Args...> &&
            std::invocable<SecondCtor, second_allocator_type&, T*, Args...>
        constexpr decltype(auto) construct(T* const ptr, Args&&... args) noexcept(
            nothrow_invocable<FirstCtor, first_allocator_type&, T*, Args...> &&
            nothrow_invocable<SecondCtor, second_allocator_type&, T*, Args...> //
        )
        {
            auto& [first, second] = alloc_pair_;
            if(first.contains(first_cvp_traits::to_pointer(static_cast<const void*>(ptr))))
                return first_traits::construct(first, ptr, cpp_forward(args)...);
            return second_traits::construct(second, ptr, cpp_forward(args)...);
        }

        [[nodiscard]] constexpr bool contains(const value_type* const ptr) const noexcept
            requires allocator_contains<SecondAlloc>
        {
            const auto vp = static_cast<const void*>(ptr);
            const auto& [first, second] = alloc_pair_;
            return first.contains(first_cvp_traits::to_pointer(vp)) ||
                second.contains(second_cvp_traits::to_pointer(vp));
        }

        [[nodiscard]] bool operator==(const composed_allocator&) const noexcept = default;

        [[nodiscard]] constexpr auto& get_allocators() const noexcept { return alloc_pair_; }

        [[nodiscard]] constexpr auto& get_allocators() noexcept { return alloc_pair_; }

        [[nodiscard]] constexpr auto& get_first_allocator() const noexcept
        {
            return alloc_pair_.first;
        }

        [[nodiscard]] constexpr auto& get_first_allocator() noexcept { return alloc_pair_.first; }

        [[nodiscard]] constexpr auto& get_second_allocator() const noexcept
        {
            return alloc_pair_.second;
        }

        [[nodiscard]] constexpr auto& get_second_allocator() noexcept { return alloc_pair_.second; }
    };

    template<typename T, typename U>
    composed_allocator(T&&, U&&) -> composed_allocator<std::decay_t<T>, std::decay_t<U>>;
}