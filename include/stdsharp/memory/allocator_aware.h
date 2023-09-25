#pragma once

#include "allocator_aware/typed_allocation.h"

namespace stdsharp
{
    template<typename T, typename Alloc>
    struct basic_allocator_aware : allocator_traits<Alloc>
    {
        using allocator_type = Alloc;
        using allocator_traits = allocator_traits<allocator_type>;
        using typename allocator_traits::size_type;
        using typename allocator_traits::const_void_pointer;

        using allocation = allocator_aware::allocation<allocator_type>;

        template<typename U>
        using typed_allocation = allocator_aware::typed_allocation<allocator_type, U>;

    private:
        constexpr auto& to_concrete() noexcept { return static_cast<T&>(*this); }

        static constexpr auto has_alloc_getter = requires(T t) {
            {
                t.get_allocator()
            } noexcept -> ::std::convertible_to<allocator_type&>;
            {
                std::as_const(t).get_allocator()
            } noexcept -> ::std::convertible_to<const allocator_type&>;
        };

        template<typename... Args, typename Fn = decltype(std::ranges::construct_at)>
            requires std::invocable<Fn, T*, Args...>
        constexpr auto ctor(Args&&... args) noexcept(nothrow_invocable<Fn, T*, Args...>)
        {
            std::ranges::construct_at(static_cast<T*>(this), cpp_forward(args)...);
        }

    public:
        basic_allocator_aware() = default;

        constexpr basic_allocator_aware(
            const std::allocator_arg_t,
            const allocator_type& alloc,
            auto&&... args
        ) noexcept(noexcept(ctor(cpp_forward(args)..., alloc)))
            requires requires { ctor(cpp_forward(args)..., alloc); }
        {
            ctor(cpp_forward(args)..., alloc);
        }

        constexpr basic_allocator_aware(const T& other) //
            noexcept(noexcept(ctor(other, other.get_allocator())))
            requires has_alloc_getter && requires {
                ctor(
                    other,
                    allocator_traits::select_on_container_copy_construction(other.get_allocator())
                );
            }
        {
            ctor(
                other,
                allocator_traits::select_on_container_copy_construction(other.get_allocator())
            );
        }

        constexpr basic_allocator_aware(T&& other) //
            noexcept(noexcept(ctor(cpp_move(other), other.get_allocator())))
            requires has_alloc_getter &&
            requires { ctor(cpp_move(other), other.get_allocator()); }
        {
            ctor(cpp_move(other), cpp_move(other).get_allocator());
        }

    protected:
        constexpr void allocate(const size_type size, const const_void_pointer hint = nullptr)
            requires has_alloc_getter
        {
            return allocator_aware::make_allocation(to_concrete().get_allocator(), size, hint);
        }

        constexpr void deallocate(allocation& alloc) noexcept
            requires has_alloc_getter
        {
            alloc.deallocate(to_concrete().get_allocator());
        }

        template<typename ValueT, typename... Args>
        [[nodiscard]] constexpr auto construct(Args&&... args)
            requires requires {
                requires has_alloc_getter;
                requires ::std::invocable<
                    allocator_aware::make_typed_allocation_fn<ValueT>,
                    allocator_type&,
                    Args... // clang-format off
                >; // clang-format on
            }
        {
            return allocator_aware::make_typed_allocation<ValueT>(
                to_concrete().get_allocator(),
                cpp_forward(args)...
            );
        }

        template<typename ValueT, typename... Args>
        [[nodiscard]] constexpr auto construct(allocation& hint, Args&&... args)
            requires requires {
                requires has_alloc_getter;
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
                to_concrete().get_allocator(),
                cpp_forward(args)...
            );
        }

        template<typename ValueT>
        constexpr void destroy(typed_allocation<ValueT>& allocation) //
            noexcept(noexcept(allocation.destroy(to_concrete().get_allocator())))
            requires requires {
                requires has_alloc_getter;
                allocation.destroy(to_concrete().get_allocator());
            }
        {
            allocation.destroy(to_concrete().get_allocator());
        }
    };
}