#pragma once

#include "allocation.h"
#include "allocation_traits.h"

namespace stdsharp::allocator_aware
{
    template<allocator_req Allocator, typename ValueType>
    class [[nodiscard]] typed_allocation
    {
    public:
        using allocator_type = Allocator;
        using value_type = ValueType;
        using traits = allocator_traits<allocator_type>;
        using cvp = typename traits::const_void_pointer;

    private:
        static constexpr auto propagate_on_copy_v = traits::propagate_on_copy_v;
        static constexpr auto propagate_on_move_v = traits::propagate_on_move_v;
        static constexpr auto always_equal_v = traits::always_equal_v;

        using allocation_type = allocation<allocator_type>;
        using allocation_traits = allocation_traits<allocation_type>;

    public:
        static constexpr auto destructible = traits::template destructible<value_type>;

        static constexpr auto destructible_req =
            get_expr_req(destructible, traits::template nothrow_destructible<value_type>);

        static constexpr auto mov_constructible = true;

        static constexpr auto mov_construct_req = expr_req::no_exception;

        static constexpr auto cp_constructible = traits::template cp_constructible<value_type>;

        static constexpr auto cp_construct_req = get_expr_req(cp_constructible);

        static constexpr auto cp_assignable = cp_constructible && destructible;

        static constexpr auto cp_assign_req = std::min( //
            {
                cp_construct_req,
                destructible_req,
                get_expr_req(true, !copy_assignable<value_type> || nothrow_copy_assignable<value_type>),
                get_expr_req(true, !propagate_on_copy_v || always_equal_v) //
            }
        );

    private:
        static constexpr auto mov_allocation_v = always_equal_v || propagate_on_move_v;

    public:
        static constexpr auto mov_assignable =
            mov_allocation_v || traits::template mov_constructible<value_type> && destructible;

        static constexpr auto mov_assign_req = mov_allocation_v ?
            expr_req::no_exception :
            std::min( //
                {
                    get_expr_req(
                        traits::template mov_constructible<value_type>,
                        traits::template nothrow_mov_constructible<value_type> //
                    ),
                    destructible_req,
                    get_expr_req(true, !move_assignable<value_type> || nothrow_move_assignable<value_type>) //
                }
            );

        static constexpr auto swappable = true;

        static constexpr auto swappable_req = expr_req::no_exception;

        static constexpr special_mem_req operation_constraints{
            mov_construct_req,
            cp_construct_req,
            mov_assign_req,
            cp_assign_req,
            destructible_req,
            swappable_req //
        };

    private:
        allocation_type allocation_{};
        bool has_value_ = false;

    public:
        typed_allocation() = default;
        typed_allocation(const typed_allocation&) = default;
        typed_allocation(typed_allocation&&) = default;
        typed_allocation& operator=(const typed_allocation&) = default;
        typed_allocation& operator=(typed_allocation&&) = default;

        constexpr ~typed_allocation() noexcept { Expects(!has_value()); }

        constexpr typed_allocation(
            const allocation_type& allocation,
            const bool has_value = false
        ) noexcept:
            allocation_(allocation), has_value_(has_value)
        {
            const auto size = allocation_.size();
            Expects(size == 0 ? !has_value_ : (size >= sizeof(value_type)));
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

        constexpr void allocate(allocator_type& alloc, const cvp hint = nullptr)
        {
            allocation_traits::allocate(allocation_, alloc, sizeof(value_type), hint);
        }

        constexpr void deallocate(allocator_type& alloc) noexcept(is_noexcept(destructible_req))
            requires destructible
        {
            Expects(!has_value_);
            allocation_.deallocate(alloc);
        }

        constexpr void shrink_to_fit(allocator_type& alloc)
            requires(traits::template constructible_from<value_type, value_type>)
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
        constexpr value_type& construct(allocator_type& alloc, Args&&... args) noexcept( //
            noexcept( //
                allocation_traits:: //
                template construct<value_type>(allocation_, alloc, cpp_forward(args)...)
            )
        )
            requires requires {
                allocation_traits::template construct<value_type>(
                    allocation_,
                    alloc,
                    cpp_forward(args)...
                );
            }
        {
            Expects(!has_value_);

            allocation_traits::template construct<value_type>(
                allocation_,
                alloc,
                cpp_forward(args)...
            );
            has_value_ = true;

            return get();
        }

        constexpr void destroy(allocator_type& alloc) noexcept(is_noexcept(destructible_req))
            requires destructible
        {
            if(!has_value()) return;
            allocation_traits::template destroy<value_type>(allocation_, alloc);
            has_value_ = false;
        }

        [[nodiscard]] constexpr auto& allocation() const noexcept { return allocation_; }

        [[nodiscard]] constexpr auto cp_construct(allocator_type& alloc) const //
            noexcept(is_noexcept(cp_construct_req))
            requires cp_constructible
        {
            typed_allocation allocation = make_allocation(alloc, sizeof(ValueType));
            allocation.construct(alloc, get_const());
            return allocation;
        }

        [[nodiscard]] constexpr auto mov_construct(allocator_type& /*unused*/) noexcept
        {
            return std::exchange(*this, {});
        }

    private:
        static constexpr void
            assign_impl(allocator_type& dst_alloc, typed_allocation& dst_allocation, auto&& value)
        {
            if constexpr(std::assignable_from<value_type, decltype(value)>)
                if(dst_allocation.has_value())
                {
                    dst_allocation.get() = cpp_forward(value);
                    return;
                }

            dst_allocation.destroy(dst_alloc);
            dst_allocation.construct(dst_alloc, cpp_forward(value));
        }

        template<bool Propagate>
        static constexpr void assign_on_no_value(
            allocator_type& dst_alloc,
            typed_allocation& dst_allocation,
            auto&& src_alloc
        )
        {
            if constexpr(Propagate)
            {
                dst_allocation.destroy(dst_alloc);

                if constexpr(!always_equal_v)
                    if(dst_alloc != src_alloc) dst_allocation.deallocate(dst_alloc);

                dst_alloc = cpp_forward(src_alloc);
            }
            else dst_allocation.destroy(dst_alloc);
        }

    public:
        constexpr void cp_assign(
            const allocator_type& src_alloc,
            allocator_type& dst_alloc,
            typed_allocation& dst_allocation
        ) const noexcept(is_noexcept(cp_assign_req))
            requires cp_assignable
        {
            Expects(allocation_);
            Expects(dst_allocation.allocation_);

            if(!has_value())
            {
                assign_on_no_value<propagate_on_copy_v>(dst_alloc, dst_allocation, src_alloc);
                return;
            }

            const auto& assign_fn = [&]
            {
                assign_impl(dst_alloc, dst_allocation, get_const()); //
            };

            if constexpr(propagate_on_copy_v)
                while(true)
                {
                    if constexpr(!always_equal_v)
                        if(dst_alloc != src_alloc)
                        {
                            dst_allocation.deallocate(dst_alloc);
                            dst_alloc = src_alloc;
                            dst_allocation.allocate(dst_alloc);

                            break;
                        }

                    dst_alloc = src_alloc;
                    break;
                }

            assign_fn();
        }

        constexpr void mov_assign(
            allocator_type& src_alloc,
            allocator_type& dst_alloc,
            typed_allocation& dst_allocation
        ) noexcept(is_noexcept(mov_assign_req))
            requires mov_assignable
        {
            Expects(allocation_);
            Expects(dst_allocation.allocation_);

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

            dst_allocation.destroy(dst_alloc);
            dst_allocation.deallocate(dst_alloc);
            if constexpr(propagate_on_move_v) dst_alloc = cpp_move(src_alloc);
            dst_allocation = std::exchange(*this, {});
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