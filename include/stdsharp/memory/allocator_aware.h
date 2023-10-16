#pragma once

#include "allocator_aware/typed_allocation.h"
#include "stdsharp/concepts/concepts.h"

namespace stdsharp
{

    template<typename T, allocator_req Alloc> // NOLINTBEGIN(*-noexcept-*)
    struct basic_allocator_aware : allocator_traits<Alloc>
    {
        using allocator_type = Alloc;
        using allocator_traits = allocator_traits<allocator_type>;
        using typename allocator_traits::size_type;
        using typename allocator_traits::const_void_pointer;
        using allocator_traits::select_on_container_copy_construction;

        using allocation = allocator_aware::allocation<allocator_type>;

        template<typename U>
        using typed_allocation = allocator_aware::typed_allocation<allocator_type, U>;

    private:
        constexpr T& to_concrete() noexcept { return static_cast<T&>(*this); }

        constexpr const T& to_concrete() const noexcept { return static_cast<const T&>(*this); }

        template<typename U = T>
        constexpr allocator_type& get_allocator() noexcept
            requires requires(U u) {
                {
                    u.get_allocator()
                } noexcept -> ::std::convertible_to<allocator_type&>;
            }
        {
            return to_concrete().get_allocator();
        }

        template<typename U = const T>
        constexpr const allocator_type& get_allocator() const noexcept
            requires requires(U u) {
                {
                    u.get_allocator()
                } noexcept -> ::std::convertible_to<const allocator_type&>;
            }
        {
            return to_concrete().get_allocator();
        }

    protected:
        constexpr void allocate(const size_type size, const const_void_pointer hint = nullptr)
            requires requires { get_allocator(); }
        {
            return allocator_aware::make_allocation(get_allocator(), size, hint);
        }

        constexpr void deallocate(allocation& alloc) noexcept
            requires requires { get_allocator(); }
        {
            alloc.deallocate(get_allocator());
        }

        template<typename ValueT, typename... Args>
        [[nodiscard]] constexpr auto construct(Args&&... args)
            requires requires {
                requires requires { get_allocator(); };
                requires ::std::invocable<
                    allocator_aware::make_typed_allocation_fn<ValueT>,
                    allocator_type&,
                    Args... // clang-format off
                >; // clang-format on
            }
        {
            return allocator_aware::make_typed_allocation<ValueT>(
                get_allocator(),
                cpp_forward(args)...
            );
        }

        template<typename ValueT, typename... Args>
        [[nodiscard]] constexpr auto construct(allocation& hint, Args&&... args)
            requires requires {
                requires requires { get_allocator(); };
                requires ::std::invocable<
                    allocator_aware::make_typed_allocation_fn<ValueT>,
                    allocation&,
                    allocator_type&,
                    Args... // clang-format off
                >; // clang-format on
            }
        {
            return allocator_aware::make_typed_allocation<ValueT>(
                hint,
                get_allocator(),
                cpp_forward(args)...
            );
        }

        template<typename ValueT>
        constexpr void destroy(typed_allocation<ValueT>& allocation) //
            noexcept(noexcept(allocation.destroy(get_allocator())))
            requires requires { allocation.destroy(get_allocator()); }
        {
            allocation.destroy(get_allocator());
        }
    }; // NOLINTEND(*-noexcept-*)
}