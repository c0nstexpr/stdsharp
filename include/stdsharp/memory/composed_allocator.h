#pragma once

#include "allocator_traits.h"

namespace stdsharp
{
    namespace details
    {
        template<typename T>
        concept allocator_contains = requires(
            const T alloc,
            const typename allocator_traits<T>::const_void_pointer ptr
        ) // clang-format off
        {
            { alloc.contains(ptr) } -> std::convertible_to<bool>; // clang-format on
            requires noexcept(alloc.contains(ptr));
        };
    }

    template<details::allocator_contains FirstAlloc, allocator_req SecondAlloc>
        requires std::same_as<typename FirstAlloc::value_type, typename SecondAlloc::value_type>
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
        constexpr explicit composed_allocator(Args&&... args) //
            noexcept(nothrow_constructible_from<Pair, Args...>):
            alloc_pair_(cpp_forward(args)...)
        {
        }

        constexpr auto allocate(const std::size_t n, const void* const hint = nullptr)
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

        constexpr auto try_allocate(const std::size_t n, const void* const hint = nullptr) noexcept
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

        constexpr void deallocate(value_type* const ptr, const std::size_t n)
        {
            auto& [first, second] = alloc_pair_;
            if(first.contains(first_ptr_traits::to_pointer(ptr))) first.deallocate(ptr, n);
            else second.deallocate(ptr, n);
        }

        [[nodiscard]] constexpr auto max_size() const noexcept
        {
            return std::min(
                first_traits::max_size(alloc_pair_.first),
                second_traits::max_size(alloc_pair_.second)
            );
        }

        [[nodiscard]] constexpr auto select_on_container_copy_construction() const
        {
            return composed_allocator{
                first_traits::select_on_container_copy_construction(alloc_pair_.first),
                second_traits::select_on_container_copy_construction(alloc_pair_.second) //
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

                    result = {
                        .ptr = second_ptr_traits::to_address(res.ptr),
                        .count = res.count //
                    };
                }
            );

            return result;
        }

        template<typename T, typename... Args>
            requires(first_traits::template constructible_from<T, Args...> && second_traits::template constructible_from<T, Args...>)
        constexpr decltype(auto) construct(T* const ptr, Args&&... args) //
            noexcept(
                first_traits::template nothrow_constructible_from<T, Args...> &&
                second_traits::template nothrow_constructible_from<T, Args...> //
            )
        {
            auto& [first, second] = alloc_pair_;
            if(first.contains(first_cvp_traits::to_pointer(to_void_pointer(ptr))))
                return first_traits::construct(first, ptr, cpp_forward(args)...);
            return second_traits::construct(second, ptr, cpp_forward(args)...);
        }

        constexpr bool contains(const value_type* const ptr) const noexcept
            requires details::allocator_contains<SecondAlloc>
        {
            return alloc_pair_.first.contains(first_cvp_traits::to_pointer(ptr)) ?
                alloc_pair_.second.contains(second_cvp_traits::to_pointer(ptr)) :
                false;
        }

        [[nodiscard]] bool operator==(const composed_allocator&) const noexcept = default;
    };

    template<typename T, typename U>
    composed_allocator(T&&, U&&) -> composed_allocator<std::decay_t<T>, std::decay_t<U>>;
}