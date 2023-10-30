#pragma once

#include "allocation_value.h"

namespace stdsharp::allocator_aware::details
{
}

namespace stdsharp::allocator_aware
{
    template<
        allocator_req Allocator,
        allocations_view<Allocator> Allocations,
        typename ValueType = Allocator::value_type>
        requires nothrow_copyable<allocation_value<Allocator, ValueType>>
    class basic_aa // NOLINTBEGIN(*-noexcept-*)
    {
    public:
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_type = Allocator;
        using allocator_traits = allocation_traits::allocator_traits;

    protected:
        indexed_values<
            ebo_union<allocator_type>,
            ebo_union<Allocations>,
            ebo_union<allocation_value<allocator_type, ValueType>>>
            values_{};

        constexpr auto& get_allocator() noexcept { return cpo::get_element<0>(values_).v; }

        constexpr auto& get_allocations() noexcept { return cpo::get_element<1>(values_).v; }

        constexpr auto& get_allocations() const noexcept { return cpo::get_element<1>(values_).v; }

        constexpr auto get_allocations_view() noexcept
        {
            return get_allocations() | std::views::all;
        }

        constexpr auto get_allocations_view() const noexcept
        {
            return get_allocations() | std::views::all;
        }

        constexpr auto& get_allocation_value() noexcept { return cpo::get_element<2>(values_).v; }

        constexpr auto& get_allocation_value() const noexcept
        {
            return cpo::get_element<2>(values_).v;
        }

        template<typename GetAllocator, typename GetAllocations>
        constexpr basic_aa(defer_allocations<GetAllocator, GetAllocations>&& defer):
            values_(
                std::invoke(cpp_move(defer.get_allocator)),
                std::invoke(cpp_move(defer.get_allocations))
            )
        {
        }

    public:
        constexpr const allocator_type& get_allocator() const noexcept
        {
            return cpo::get_element<0>(values_).v;
        }

        basic_aa() = default;

        constexpr basic_aa(const basic_aa& other, const allocator_type& alloc) noexcept( //
            noexcept(allocation_traits::on_construct(get_allocations(), get_allocator()))
        )
            requires requires {
                allocation_traits::on_construct(get_allocations(), get_allocator());
            }
            : basic_aa(allocation_traits::on_construct(get_allocations(), get_allocator()))
        {
        }

        constexpr basic_aa(const basic_aa& other) noexcept( //
            noexcept( //
                allocator_traits::select_on_container_copy_construction(get_allocator()),
                allocation_traits::on_construct(get_allocations(), get_allocator())
            )
        )
            requires requires {
                allocation_traits::on_construct(get_allocations(), get_allocator());
            }
            :
            allocator_type(
                allocator_traits::select_on_container_copy_construction(other.get_allocator())
            ),
            values_(allocation_traits::on_construct(other.get_allocations(), get_allocator()))
        {
        }

        constexpr basic_aa(basic_aa&& other, const allocator_type& alloc) noexcept( //
            noexcept(allocation_traits::on_construct(get_allocations(), get_allocator()))
        )
            requires requires {
                allocation_traits::on_construct(get_allocations(), get_allocator());
            }
            :
            allocator_type(alloc),
            values_(allocation_traits::on_construct(other.get_allocations(), get_allocator()))
        {
        }

        constexpr basic_aa(basic_aa&& other) noexcept( //
            noexcept(allocation_traits::on_construct(get_allocations(), get_allocator()))
        )
            requires requires {
                allocation_traits::on_construct(get_allocations(), get_allocator());
            }
            :
            allocator_type(cpp_move(other.get_allocator())),
            values_(allocation_traits::on_construct(other.get_allocations(), get_allocator()))
        {
        }

        constexpr basic_aa& operator=(const basic_aa& other) noexcept( //
            noexcept( //
                allocation_traits::on_assign(
                    get_allocations(),
                    get_allocator(),
                    get_allocations(),
                    get_allocator()
                )
            )
        )
            requires requires {
                allocation_traits::on_assign(
                    get_allocations(),
                    get_allocator(),
                    get_allocations(),
                    get_allocator()
                );
            }
        {
            if(this == &other) return *this;

            allocation_traits::on_assign(
                other.get_allocations(),
                other.get_allocator(),
                get_allocations(),
                get_allocator()
            );

            return *this;
        }

        constexpr basic_aa& operator=(basic_aa&& other) noexcept( //
            noexcept( //
                allocation_traits::on_assign(
                    get_allocations(),
                    get_allocator(),
                    get_allocations(),
                    get_allocator()
                )
            )
        )
            requires requires {
                allocation_traits::on_assign(
                    get_allocations(),
                    get_allocator(),
                    get_allocations(),
                    get_allocator()
                );
            }
        {
            if(this == &other) return *this;

            allocation_traits::on_assign(
                other.get_allocations(),
                other.get_allocator(),
                get_allocations(),
                get_allocator()
            );

            return *this;
        }

        constexpr ~basic_aa() noexcept(noexcept(deallocate())) { deallocate(); }

        constexpr void swap(basic_aa& other) noexcept( //
            noexcept( //
                allocation_traits::on_swap(
                    get_allocations(),
                    get_allocator(),
                    get_allocations(),
                    get_allocator()
                )
            )
        )
            requires requires {
                allocation_traits::on_swap(
                    get_allocations(),
                    get_allocator(),
                    get_allocations(),
                    get_allocator()
                );
            }
        {
            if(this == &other) return;

            allocation_traits::on_swap(
                get_allocations(),
                get_allocator(),
                other.get_allocations(),
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
                    allocate(get_allocations(), get_allocator(), cpp_forward(args)...);
            }
        {
            allocation_traits:: //
                allocate(get_allocations(), get_allocator(), cpp_forward(args)...);
        }

        constexpr void deallocate() //
            noexcept(noexcept(allocation_traits::deallocate(get_allocations(), get_allocator())))
        {
            allocation_traits::deallocate(get_allocations(), get_allocator());
        }

        template<typename T>
        constexpr T& construct(auto&&... args) noexcept( //
            noexcept( //
                allocation_traits:: //
                template construct<T>(get_allocations(), get_allocator(), cpp_forward(args)...)
            )
        )
        // requires requires {
        //     allocation_traits:: //
        //         template construct<T>(get_allocation(), get_allocator(), cpp_forward(args)...);
        // }
        {
            return allocation_traits::template construct<T>(
                get_allocations(),
                get_allocator(),
                cpp_forward(args)...
            );
        }

        constexpr void destroy() //
            noexcept(noexcept(allocation_traits::destroy(get_allocations(), get_allocator())))
            requires requires { allocation_traits::destroy(get_allocations(), get_allocator()); }
        {
            allocation_traits::destroy(get_allocations(), get_allocator());
        }
    }; // NOLINTEND(*-noexcept-*)
}