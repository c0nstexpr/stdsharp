#pragma once

#include "allocation_traits.h"

namespace stdsharp::allocator_aware
{
    template<typename Allocation>
        requires requires { typename allocation_traits<Allocation>; }
    class basic_aa : allocation_traits<Allocation>::allocator_type // NOLINTBEGIN(*-noexcept-*)
    {
    public:
        using allocation_traits = allocation_traits<Allocation>;
        using allocation_type = allocation_traits::allocation_type;
        using callocation = allocation_traits::callocation;
        using allocator_type = allocation_traits::allocator_type;
        using allocator_traits = allocation_traits::allocator_traits;

    private:
        allocation_type allocation_{};

    protected:
        constexpr allocation_type& get_allocation() noexcept { return allocation_; }

        constexpr const allocation_type& get_allocation() const noexcept { return allocation_; }

        constexpr allocator_type& get_allocator() noexcept { return *this; }

        template<typename T, typename U>
            requires std::constructible_from<allocator_type, T> &&
                         std::constructible_from<allocation_type, U>
        constexpr basic_aa(T&& allocator, U&& allocation) //
            noexcept(nothrow_constructible_from<allocator_type, T> && nothrow_constructible_from<allocation_type, U>):
            allocator_type(cpp_forward(allocator)), allocation_(cpp_forward(allocation))
        {
        }

        template<typename T, invocable_r<allocation_type, allocator_type> U>
            requires std::constructible_from<allocator_type, T>
        constexpr basic_aa(T&& allocator, U&& deferred_allocation) //
            noexcept(nothrow_constructible_from<allocator_type, T> && nothrow_invocable_r<U, allocation_type, allocator_type>):
            allocator_type(cpp_forward(allocator)),
            allocation_(std::invoke_r(cpp_forward(deferred_allocation), get_allocator()))
        {
        }

    public:
        constexpr const allocator_type& get_allocator() const noexcept { return *this; }

        basic_aa() = default;

        constexpr basic_aa(const basic_aa& other, const allocator_type& alloc) noexcept( //
            noexcept(allocation_traits::cp_construct(get_allocation(), get_allocator()))
        )
            requires requires {
                allocation_traits::cp_construct(get_allocation(), get_allocator());
            }
            :
            allocator_type(alloc),
            allocation_(allocation_traits::cp_construct(other.get_allocation(), get_allocator()))
        {
        }

        constexpr basic_aa(const basic_aa& other) noexcept( //
            noexcept( //
                allocator_traits::select_on_container_copy_construction(get_allocator()),
                allocation_traits::cp_construct(get_allocation(), get_allocator())
            )
        )
            requires requires {
                allocation_traits::cp_construct(get_allocation(), get_allocator());
            }
            :
            allocator_type(
                allocator_traits::select_on_container_copy_construction(other.get_allocator())
            ),
            allocation_(allocation_traits::cp_construct(other.get_allocation(), get_allocator()))
        {
        }

        constexpr basic_aa(basic_aa&& other, const allocator_type& alloc) noexcept( //
            noexcept(allocation_traits::mov_construct(get_allocation(), get_allocator()))
        )
            requires requires {
                allocation_traits::mov_construct(get_allocation(), get_allocator());
            }
            :
            allocator_type(alloc),
            allocation_(allocation_traits::mov_construct(other.get_allocation(), get_allocator()))
        {
        }

        constexpr basic_aa(basic_aa&& other) noexcept( //
            noexcept(allocation_traits::mov_construct(get_allocation(), get_allocator()))
        )
            requires requires {
                allocation_traits::mov_construct(get_allocation(), get_allocator());
            }
            :
            allocator_type(cpp_move(other.get_allocator())),
            allocation_(allocation_traits::mov_construct(other.get_allocation(), get_allocator()))
        {
        }

        constexpr basic_aa& operator=(const basic_aa& other) noexcept( //
            noexcept( //
                allocation_traits::cp_assign(
                    get_allocation(),
                    get_allocator(),
                    get_allocation(),
                    get_allocator()
                )
            )
        )
            requires requires {
                allocation_traits::cp_assign(
                    get_allocation(),
                    get_allocator(),
                    get_allocation(),
                    get_allocator()
                );
            }
        {
            if(this == &other) return *this;

            allocation_traits::cp_assign(
                other.get_allocation(),
                other.get_allocator(),
                get_allocation(),
                get_allocator()
            );

            return *this;
        }

        constexpr basic_aa& operator=(basic_aa&& other) noexcept( //
            noexcept( //
                allocation_traits::mov_assign(
                    get_allocation(),
                    get_allocator(),
                    get_allocation(),
                    get_allocator()
                )
            )
        )
            requires requires {
                allocation_traits::mov_assign(
                    get_allocation(),
                    get_allocator(),
                    get_allocation(),
                    get_allocator()
                );
            }
        {
            if(this == &other) return *this;

            allocation_traits::mov_assign(
                other.get_allocation(),
                other.get_allocator(),
                get_allocation(),
                get_allocator()
            );

            return *this;
        }

        constexpr ~basic_aa() noexcept(noexcept(deallocate())) { deallocate(); }

        constexpr void swap(basic_aa& other) noexcept( //
            noexcept( //
                allocation_traits::swap(
                    get_allocation(),
                    get_allocator(),
                    get_allocation(),
                    get_allocator()
                )
            )
        )
            requires requires {
                allocation_traits::swap(
                    get_allocation(),
                    get_allocator(),
                    get_allocation(),
                    get_allocator()
                );
            }
        {
            if(this == &other) return;

            allocation_traits::swap(
                get_allocation(),
                get_allocator(),
                other.get_allocation(),
                other.get_allocator()
            );
        }

        constexpr explicit basic_aa(const allocator_type& allocator) noexcept:
            allocator_type(allocator)
        {
        }

    protected:
        constexpr void allocate(auto&&... args)
            requires requires {
                allocation_traits:: //
                    allocate(get_allocation(), get_allocator(), cpp_forward(args)...);
            }
        {
            allocation_traits:: //
                allocate(get_allocation(), get_allocator(), cpp_forward(args)...);
        }

        constexpr void deallocate() //
            noexcept(noexcept(allocation_traits::deallocate(get_allocation(), get_allocator())))
        {
            allocation_traits::deallocate(get_allocation(), get_allocator());
        }

        template<typename T>
        constexpr T& construct(auto&&... args) noexcept( //
            noexcept( //
                allocation_traits:: //
                template construct<T>(get_allocation(), get_allocator(), cpp_forward(args)...)
            )
        )
            // requires requires {
            //     allocation_traits:: //
            //         template construct<T>(get_allocation(), get_allocator(), cpp_forward(args)...);
            // }
        {
            return allocation_traits::template construct<T>(
                get_allocation(),
                get_allocator(),
                cpp_forward(args)...
            );
        }

        constexpr void destroy() //
            noexcept(noexcept(allocation_traits::destroy(get_allocation(), get_allocator())))
            requires requires { allocation_traits::destroy(get_allocation(), get_allocator()); }
        {
            allocation_traits::destroy(get_allocation(), get_allocator());
        }
    }; // NOLINTEND(*-noexcept-*)
}