#pragma once

#include "allocation_value_operation.h"

namespace stdsharp::allocator_aware
{
    template<
        allocator_req Allocator,
        typename Allocations,
        typename ValueType = Allocator::value_type>
    class basic_aa :
        Allocator,
        allocation_value_operation<Allocator, ValueType> // NOLINTBEGIN(*-noexcept-*)
    {
    public:
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_type = Allocator;
        using allocator_traits = allocation_traits::allocator_traits;

    private:
        Allocations allocations_{};

    protected:
        constexpr auto& get_allocation() noexcept { return allocations_; }

        constexpr const auto& get_allocation() const noexcept { return allocations_; }

        constexpr allocator_type& get_allocator() noexcept { return *this; }

        template<typename Fn, typename Result = std::invoke_result_t<Fn, allocator_type&>>
        constexpr basic_aa(ctor_input_allocations<allocator_type, Fn>&& input) //
            noexcept(nothrow_invocable<Fn, allocator_type&> && nothrow_constructible_from<Allocations, Result>):
            allocator_type(cpp_move(input.allocator)),
            allocations_(std::invoke(cpp_move(input.deferred_allocations), get_allocator()))
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
            allocations_(allocation_traits::cp_construct(other.get_allocation(), get_allocator()))
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
            allocations_(allocation_traits::cp_construct(other.get_allocation(), get_allocator()))
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
            allocations_(allocation_traits::mov_construct(other.get_allocation(), get_allocator()))
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
            allocations_(allocation_traits::mov_construct(other.get_allocation(), get_allocator()))
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