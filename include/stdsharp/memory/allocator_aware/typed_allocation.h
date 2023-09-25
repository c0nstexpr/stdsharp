#pragma once

#include "allocation.h"

namespace stdsharp::allocator_aware
{
    struct bad_allocation_construct_call : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    template<allocator_req Allocator, typename ValueType>
    class [[nodiscard]] typed_allocation

    {
    public:
        using allocator_type = Allocator;
        using value_type = ValueType;
        using traits = allocator_traits<allocator_type>;
        using const_void_pointer = typename traits::const_void_pointer;

    private:
        static constexpr auto propagate_on_copy_v = traits::propagate_on_copy_v;
        static constexpr auto propagate_on_move_v = traits::propagate_on_move_v;
        static constexpr auto always_equal_v = traits::always_equal_v;

    public:
        static constexpr auto mov_constructible = true;

        static constexpr auto mov_construct_req = expr_req::no_exception;

        static constexpr auto cp_constructible = traits::template cp_constructible<value_type>;

        static constexpr auto cp_construct_req = get_expr_req(cp_constructible);

        static constexpr auto cp_assignable = cp_constructible && copy_assignable<value_type>;

        static constexpr auto cp_assign_req = std::min(
            cp_construct_req,
            get_expr_req(copy_assignable<value_type>, !propagate_on_copy_v || always_equal_v)
        );

    private:
        static constexpr auto mov_allocation_v = !(always_equal_v || propagate_on_move_v);

    public:
        static constexpr auto mov_assignable = mov_allocation_v ||
            traits::template mov_constructible<value_type> && move_assignable<value_type>;

        static constexpr auto mov_assign_req = mov_allocation_v ?
            expr_req::no_exception :
            std::min(
                get_expr_req(
                    traits::template mov_constructible<value_type>,
                    traits::template nothrow_mov_constructible<value_type> //
                ),
                get_expr_req(move_assignable<value_type>, nothrow_move_assignable<value_type>)
            );

        static constexpr auto destructible = traits::template destructible<value_type>;

        static constexpr auto destructible_req =
            get_expr_req(destructible, traits::template nothrow_destructible<value_type>);

        static constexpr auto swappable =
            requires(allocator_type alloc, typed_allocation allocation) {
                allocation.swap(alloc, alloc, allocation);
            };

        static constexpr auto swappable_req = get_expr_req(
            swappable,
            requires(allocator_type alloc, typed_allocation allocation) {
                {
                    allocation.swap(alloc, alloc, allocation)
                } noexcept;
            }
        );

        static constexpr special_mem_req operation_constraints{
            expr_req::no_exception,
            cp_construct_req,
            mov_assign_req,
            cp_assign_req,
            expr_req::no_exception,
            destructible_req //
        };

    private:
        allocation<allocator_type> allocation_{};
        bool has_value_ = false;

    public:
        typed_allocation() = default;

        constexpr typed_allocation(
            const allocation<allocator_type>& allocation,
            const bool has_value = false
        ) noexcept(!is_debug):
            allocation_(allocation), has_value_(has_value)
        {
            precondition<std::invalid_argument>( //
                std::bind_front(
                    std::ranges::greater_equal{},
                    allocation_.size(),
                    has_value_ ? sizeof(value_type) : 0
                ),
                "allocation size is too small for the value type"
            );
        }

        [[nodiscard]] constexpr auto ptr() const noexcept
        {
            return pointer_cast<value_type>(allocation_.begin());
        }

        [[nodiscard]] constexpr auto cptr() const noexcept
        {
            return pointer_cast<value_type>(allocation_.cbegin());
        }

        [[nodiscard]] constexpr decltype(auto) get() const noexcept { return *ptr(); }

        [[nodiscard]] constexpr decltype(auto) get_const() const noexcept { return *cptr(); }

        [[nodiscard]] constexpr bool has_value() const noexcept { return has_value_; }

        constexpr void allocate(allocator_type& alloc, const const_void_pointer hint = nullptr)
        {
            destroy(alloc);
            allocation_.allocate(alloc, sizeof(value_type), hint);
        }

        constexpr void deallocate(allocator_type& alloc) noexcept
        {
            if(!allocation_) return;
            destroy(alloc);
            allocation_.deallocate(alloc);
        }

        constexpr void shrink_to_fit(allocator_type& alloc)
            requires(
                traits::template construct_req<value_type, value_type> >= expr_req::well_formed
            )
        {
            if(allocation_.size() <= sizeof(value_type)) return;

            auto new_allocation = make_allocation(alloc, sizeof(value_type));

            if(has_value())
                traits::construct(
                    alloc,
                    pointer_cast<value_type>(new_allocation.begin()),
                    cpp_move(get())
                );

            deallocate(alloc);
            allocation_ = new_allocation;
        }

        template<typename... Args>
            requires(traits::template constructible_from<value_type, Args...>)
        constexpr void construct(allocator_type& alloc, Args&&... args) //
            noexcept(traits::template nothrow_constructible_from<value_type, Args...>)
        {
            precondition<bad_allocation_construct_call>(
                [this] { return allocation_.size() >= sizeof(value_type); },
                "no allocation owned when constructing the value type"
            );

            destroy(alloc);
            traits::construct(alloc, ptr(), cpp_forward(args)...);
            has_value_ = true;
        }

        constexpr void destroy(allocator_type& alloc) //
            noexcept(destructible_req >= expr_req::no_exception)
            requires destructible
        {
            if(!has_value()) return;
            traits::destroy(alloc, ptr());
            has_value_ = false;
        }

        [[nodiscard]] constexpr auto& allocation() const noexcept { return allocation_; }

        [[nodiscard]] constexpr auto cp_construct(allocator_type& alloc
        ) noexcept(cp_construct_req >= expr_req::no_exception)
            requires cp_constructible
        {
            typed_allocation<allocator_type, ValueType> allocation =
                make_allocation(alloc, sizeof(ValueType));
            allocation.construct(alloc, get_const());
            return allocation;
        }

        [[nodiscard]] constexpr auto mov_construct(allocator_type&) noexcept
        {
            return std::exchange(*this, {});
        }

    private:
        constexpr void assign_impl(
            allocator_type& dst_alloc,
            typed_allocation<allocator_type, ValueType>& dst_allocation,
            auto&& value
        )
        {
            if(dst_allocation.has_value()) dst_allocation.get() = cpp_forward(value);
            else dst_allocation.construct(dst_alloc, cpp_forward(value));
        }

        template<bool Propagate>
        constexpr void assign_on_no_value(
            allocator_type& dst_alloc,
            typed_allocation<allocator_type, ValueType>& dst_allocation,
            auto&& src_alloc
        )
        {
            if constexpr(Propagate)
            {
                [&]
                {
                    if constexpr(!always_equal_v)
                        if(dst_alloc != src_alloc)
                        {
                            dst_allocation.deallocate(dst_alloc);
                            return;
                        }

                    dst_allocation.destroy(dst_alloc);
                }();

                dst_alloc = src_alloc;
            }
            else dst_allocation.destroy(dst_alloc);
        }

    public:
        constexpr void cp_assign(
            const allocator_type& src_alloc,
            allocator_type& dst_alloc,
            typed_allocation& dst_allocation
        ) noexcept(cp_assign_req >= expr_req::no_exception)
            requires cp_assignable
        {
            if(!has_value())
            {
                assign_on_no_value<propagate_on_copy_v>(dst_alloc, dst_allocation, src_alloc);
                return;
            }

            const auto& assign_fn = [&]
            {
                assign_impl(dst_alloc, dst_allocation, get_const()); //
            };

            if constexpr(propagate_on_copy_v && !always_equal_v)
                if(dst_alloc != src_alloc)
                {
                    dst_allocation.deallocate(dst_alloc);
                    dst_alloc = src_alloc;
                    dst_allocation.allocate(dst_alloc);
                    assign_fn();

                    return;
                }

            if constexpr(propagate_on_copy_v) dst_alloc = src_alloc;
            assign_fn();
        }

        constexpr void mov_assign(
            allocator_type& src_alloc,
            allocator_type& dst_alloc,
            typed_allocation& dst_allocation
        ) noexcept(mov_assign_req >= expr_req::no_exception)
            requires mov_assignable
        {
            if(!has_value())
            {
                assign_on_no_value<propagate_on_move_v>(dst_alloc, dst_allocation, src_alloc);
                return;
            }

            if constexpr(!always_equal_v && !propagate_on_move_v)
                if(dst_alloc != src_alloc)
                {
                    assign_impl(dst_alloc, dst_allocation, cpp_move(get()));
                    return;
                }

            dst_allocation.deallocate(dst_alloc);
            if constexpr(propagate_on_move_v) dst_alloc = cpp_move(src_alloc);
            dst_allocation = std::exchange(*this, {});
        }

        constexpr void swap(
            allocator_type& src_alloc,
            allocator_type& dst_alloc,
            typed_allocation& dst_allocation
        ) noexcept(swappable_req >= expr_req::no_exception) // NOLINT(*-noexcept-swap)
            requires swappable
        {
            std::swap(has_value_, dst_allocation.has_value_);
            allocation_.swap(src_alloc, dst_alloc, dst_allocation);
        }
    };

    template<typename T>
    struct make_typed_allocation_fn
    {
    private:
        template<
            allocator_req allocator_type,
            typename... Args,
            typename Typed = typed_allocation<allocator_type, T> // clang-format off
        > // clang-format on
        [[nodiscard]] static constexpr Typed impl(
            const allocation<allocator_type>& allocation,
            allocator_type& alloc,
            Args&&... args
        )
            requires requires(Typed result) { result.construct(alloc, cpp_forward(args)...); }
        {
            Typed result{allocation};
            result.construct(alloc, cpp_forward(args)...);
            return result;
        }

    public:
        template<allocator_req allocator_type, typename... Args>
        [[nodiscard]] constexpr auto operator()(allocator_type& alloc, Args&&... args) const
            requires requires(allocation<allocator_type> allocation) {
                impl(allocation, alloc, cpp_forward(args)...);
            }
        {
            return impl(make_allocation(alloc, sizeof(T)), alloc, cpp_forward(args)...);
        }

        template<allocator_req allocator_type, typename... Args>
        [[nodiscard]] constexpr auto operator()(
            allocation<allocator_type>& hint,
            allocator_type& alloc,
            Args&&... args //
        ) const
            requires requires { impl(hint, alloc, cpp_forward(args)...); }
        {
            hint.allocate(alloc, sizeof(T));
            return impl(hint, alloc, cpp_forward(args)...);
        }
    };

    template<typename T>
    inline constexpr make_typed_allocation_fn<T> make_typed_allocation{};

    template<allocator_req Alloc, typename ValueType = Alloc::value_type>
    static constexpr auto allocation_constraints =
        typed_allocation<Alloc, ValueType>::operation_constraints;
}