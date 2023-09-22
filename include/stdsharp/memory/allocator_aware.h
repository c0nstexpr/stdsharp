#pragma once

#include <memory>
#include <span>

#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../cassert/cassert.h"
#include "stdsharp/type_traits/core_traits.h"

namespace stdsharp
{
    template<allocator_req>
    struct allocator_aware_traits;

    template<allocator_req Alloc>
    using allocation = allocator_aware_traits<Alloc>::allocation;

    template<allocator_req Alloc, typename ValueType = Alloc::value_type>
    using typed_allocation = allocator_aware_traits<Alloc>::template typed_allocation<ValueType>;

    template<allocator_req Alloc, typename ValueType = Alloc::value_type>
    static constexpr auto allocation_constraints =
        typed_allocation<Alloc, ValueType>::operation_constraints;

    namespace details
    {
        struct allocation_access
        {
            template<typename allocator_type>
            static constexpr allocation<allocator_type> make_allocation(
                allocator_type& alloc,
                const allocator_size_type<allocator_type> size,
                const allocator_cvp<allocator_type> hint = nullptr
            )
            {
                if(size == 0) [[unlikely]]
                    return {};

                return {allocator_traits<allocator_type>::allocate(alloc, size, hint), size};
            }

            template<typename allocator_type>
            static constexpr allocation<allocator_type> try_make_allocation(
                allocator_type& alloc,
                const allocator_size_type<allocator_type> size,
                const allocator_cvp<allocator_type> hint = nullptr
            ) noexcept
            {
                if(size == 0) [[unlikely]]
                    return {};

                return {allocator_traits<allocator_type>::try_allocate(alloc, size, hint), size};
            }
        };
    }

    template<allocator_req allocator_type>
    constexpr allocation<allocator_type> make_allocation(
        allocator_type& alloc,
        const allocator_size_type<allocator_type> size,
        const allocator_cvp<allocator_type> hint = nullptr
    )
    {
        return details::allocation_access::make_allocation(alloc, size, hint);
    }

    template<allocator_req allocator_type>
    constexpr allocation<allocator_type> try_make_allocation(
        allocator_type& alloc,
        const allocator_size_type<allocator_type> size,
        const allocator_cvp<allocator_type> hint = nullptr
    ) noexcept
    {
        return details::allocation_access::try_make_allocation(alloc, size, hint);
    }

    template<typename T>
    struct make_typed_allocation_fn
    {
        template<allocator_req allocator_type, typename... Args>
        [[nodiscard]] constexpr typed_allocation<allocator_type, T>
            operator()(allocator_type& alloc, Args&&... args)
        {
            using traits = allocator_traits<allocator_type>;

            const auto& allocation = make_allocation(alloc, sizeof(T));
            traits::construct(alloc, pointer_cast<T>(allocation.begin()), cpp_forward(args)...);
            return allocation;
        }
    };

    template<typename T>
    inline constexpr make_typed_allocation_fn<T> make_typed_allocation{};

    template<allocator_req Alloc>
    struct allocator_aware_traits : allocator_traits<Alloc>
    {
        using traits = allocator_traits<Alloc>;

        using typename traits::value_type;
        using typename traits::pointer;
        using typename traits::const_pointer;
        using typename traits::const_void_pointer;
        using typename traits::size_type;
        using typename traits::difference_type;
        using typename traits::allocator_type;

        using traits::propagate_on_copy_v;
        using traits::propagate_on_move_v;
        using traits::propagate_on_swap_v;
        using traits::always_equal_v;

        class allocation;

        template<typename ValueType = value_type>
        class typed_allocation;
    };

    template<allocator_req Allocator>
    class [[nodiscard]] allocator_aware_traits<Allocator>::allocation
    {
        friend details::allocation_access;

        pointer ptr_ = nullptr;
        size_type size_ = 0;

    protected:
        constexpr allocation(pointer ptr, const size_type size) noexcept: ptr_(ptr), size_(size) {}

    public:
        allocation() = default;

        [[nodiscard]] constexpr auto begin() const noexcept { return ptr_; }

        [[nodiscard]] constexpr auto end() const noexcept { return ptr_ + size_; }

        [[nodiscard]] constexpr const_pointer cbegin() const noexcept { return begin(); }

        [[nodiscard]] constexpr const_pointer cend() const noexcept { return end(); }

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        [[nodiscard]] constexpr auto empty() const noexcept { return ptr_ != nullptr; }

        constexpr operator bool() const noexcept { return empty(); }

        constexpr void allocate(
            allocator_type& alloc,
            const size_type size,
            const allocator_cvp<allocator_type> hint = nullptr
        )
        {
            if(size_ >= size) return;

            if(ptr_ != nullptr) deallocate(alloc);

            *this = make_allocation(alloc, size, hint);
        }

        constexpr void deallocate(allocator_type& alloc) noexcept
        {
            traits::deallocate(alloc, ptr_, size_);
            *this = {};
        }

        constexpr void swap(
            allocator_type& src_alloc,
            allocator_type& dst_alloc,
            allocation& dst_allocation
        ) noexcept(!is_debug) // NOLINT(*-noexcept-swap)
        {
            precondition<std::invalid_argument>( //
                [&dst_alloc, &src_alloc]
                {
                    if constexpr(!always_equal_v)
                        if(dst_alloc != src_alloc) return false;

                    return true;
                }
            );
            std::swap(dst_allocation, *this);
            if constexpr(propagate_on_swap_v) std::ranges::swap(dst_alloc, src_alloc);
        }
    };

    template<allocator_req Allocator>
    template<typename ValueType>
    class [[nodiscard]] allocator_aware_traits<Allocator>::typed_allocation
    {
    public:
        using value_type = ValueType;

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
        allocation allocation_{};
        bool has_value_ = false;

    public:
        typed_allocation() = default;

        constexpr typed_allocation(
            const allocation& allocation,
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

        [[nodiscard]] constexpr auto cp_construct(allocator_type& alloc)
            noexcept(cp_construct_req >= expr_req::no_exception)
            requires cp_constructible
        {
            typed_allocation<ValueType> allocation = make_allocation(alloc, sizeof(ValueType));
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
            typed_allocation<ValueType>& dst_allocation,
            auto&& value
        )
        {
            if(dst_allocation.has_value()) dst_allocation.get() = cpp_forward(value);
            else dst_allocation.construct(dst_alloc, cpp_forward(value));
        }

        template<bool Propagate>
        constexpr void assign_on_no_value(
            allocator_type& dst_alloc,
            typed_allocation<ValueType>& dst_allocation,
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

    template<typename T, typename Alloc>
    struct basic_allocator_aware : allocator_aware_traits<Alloc>
    {
        using allocator_type = Alloc;
        using allocator_traits = allocator_aware_traits<allocator_type>;

    private:
        constexpr auto& to_concrete() noexcept { return static_cast<T&>(*this); }

        template<typename... Args>
        constexpr auto ctor(Args&&... args)
        {
            std::ranges::construct_at(static_cast<T*>(this), cpp_forward(args)...);
        }

    public:
        basic_allocator_aware() = default;

        template<typename... Args>
            requires std::is_constructible_v<T, Args..., allocator_type>
        constexpr basic_allocator_aware(
            const std::allocator_arg_t,
            const allocator_type& alloc,
            Args&&... args
        ) noexcept(nothrow_constructible_from<T, Args..., allocator_type>)
        {
            ctor(cpp_forward(args)..., alloc);
        }

        constexpr basic_allocator_aware(const T& other) //
            noexcept(nothrow_constructible_from<T, const T&, allocator_type>)
            requires std::is_constructible_v<T, const T&, allocator_type> && requires {
                {
                    other.get_allocator()
                } noexcept -> std::convertible_to<const Alloc&>;
            }
        {
            ctor( //
                static_cast<const T&>(other),
                allocator_traits::select_on_container_copy_construction(other.get_allocator())
            );
        }

        constexpr basic_allocator_aware(T&& other) // NOLINTBEGIN(*-noexcept-move*)
            noexcept(nothrow_constructible_from<T, T, allocator_type>)
            requires std::is_constructible_v<T, T, allocator_type> && requires {
                {
                    other.get_allocator()
                } noexcept -> std::convertible_to<Alloc&>;
            }
        {
            ctor(static_cast<T&&>(other), other.get_allocator());
        }
    };
}