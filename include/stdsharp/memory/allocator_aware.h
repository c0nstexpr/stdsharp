#pragma once

#include <span>

#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../cassert/cassert.h"

namespace stdsharp
{
    template<allocator_req>
    struct allocator_aware_traits;

    template<allocator_req Alloc>
    using allocation = allocator_aware_traits<Alloc>::allocation;

    template<allocator_req Alloc, typename ValueType = Alloc::value_type>
    using allocation_for = allocator_aware_traits<Alloc>::template allocation_for<ValueType>;

    namespace details
    {
        struct allocation_access
        {
            template<typename allocator_type>
            static constexpr allocation<allocator_type> make_allocation(
                allocator_type& alloc,
                const allocator_size_type<allocator_type> size,
                const allocator_cvp<allocator_type>& hint = nullptr
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
                const allocator_cvp<allocator_type>& hint = nullptr
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
        const allocator_cvp<allocator_type>& hint = nullptr
    )
    {
        return details::allocation_access::make_allocation(alloc, size, hint);
    }

    template<allocator_req allocator_type>
    constexpr allocation<allocator_type> try_make_allocation(
        allocator_type& alloc,
        const allocator_size_type<allocator_type> size,
        const allocator_cvp<allocator_type>& hint = nullptr
    ) noexcept
    {
        return details::allocation_access::try_make_allocation(alloc, size, hint);
    }

    template<typename T>
    struct make_allocation_by_obj_fn
    {
        template<allocator_req allocator_type, typename... Args>
        [[nodiscard]] constexpr allocation_for<allocator_type, T>
            operator()(allocator_type& alloc, Args&&... args)
        {
            using traits = allocator_traits<allocator_type>;

            const auto& allocation = make_allocation(alloc, sizeof(T));
            traits::construct(alloc, pointer_cast<T>(allocation.begin()), cpp_forward(args)...);
            return allocation;
        }
    };

    template<typename T>
    inline constexpr make_allocation_by_obj_fn<T> make_allocation_by_obj{};

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
        class allocation_for;

        template<typename ValueType>
            requires(allocation_for<ValueType>::copy_constructible_req >= expr_req::well_formed)
        static constexpr allocation_for<ValueType>
            copy_construct(allocator_type& alloc, const allocation_for<ValueType>& other)
        {
            allocation_for<ValueType> allocation = make_allocation(alloc, sizeof(ValueType));
            allocation.construct(alloc, other.get_const());
            return allocation;
        }

        template<typename ValueType>
        static constexpr allocation_for<ValueType>
            move_construct(allocator_type&, allocation_for<ValueType>& other) noexcept
        {
            return ::std::exchange(other, {});
        }

        template<typename ValueType>
        static constexpr void
            destroy(allocator_type& alloc, allocation_for<ValueType>& allocation) noexcept
        {
            allocation.destroy(alloc);
        }

    private:
        template<typename ValueType>
        static constexpr void assign_impl(
            allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            auto&& value
        )
        {
            if(dst_allocation.has_value())
            {
                dst_allocation.get() = cpp_forward(value);
                return;
            }

            dst_allocation.construct(dst_alloc, cpp_forward(value));
        }

        template<typename ValueType>
        static constexpr void copy_impl(
            allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            const allocation_for<ValueType>& src_allocation
        )
        {
            if(src_allocation.has_value())
                assign_impl(dst_alloc, dst_allocation, src_allocation.get_const());
            else dst_allocation.destroy(dst_alloc);
        }

    public:
        template<typename ValueType>
            requires(allocation_for<ValueType>::copy_assignable_req >= expr_req::well_formed)
        static constexpr void copy_assign(
            allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            const allocator_type& src_alloc,
            const allocation_for<ValueType>& src_allocation
        ) noexcept(allocation_for<ValueType>::copy_assignable_req >= expr_req::no_exception)
        {
            if constexpr(propagate_on_copy_v)
                [&]
                {
                    if constexpr(!always_equal_v)
                        if(dst_alloc != src_alloc)
                        {
                            dst_allocation.deallocate(dst_alloc);
                            dst_alloc = src_alloc;
                            return;
                        }

                    dst_alloc = src_alloc;
                }();

            copy_impl(dst_alloc, dst_allocation, src_allocation);
        }

    private:
        template<typename ValueType>
        static constexpr void move_assign_impl(
            allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            allocator_type& src_alloc,
            allocation_for<ValueType>& src_allocation
        )
        {
            if(src_allocation.has_value())
            {
                assign_impl(dst_alloc, dst_allocation, cpp_move(src_allocation.get()));
                destroy(src_allocation, src_alloc);
            }
            else destroy(dst_alloc, dst_allocation);
        }

        template<typename ValueType>
            requires propagate_on_move_v
        static constexpr void
            move_assign_impl(allocator_type& dst_alloc, allocation_for<ValueType>& dst_allocation, allocator_type&, allocation_for<ValueType>&)
        {
            dst_allocation.deallocate(dst_alloc);
        }

    public:
        template<typename ValueType>
            requires(allocation_for<ValueType>::move_assignable_req >= expr_req::well_formed)
        static constexpr void move_assign(
            allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            allocator_type& src_alloc,
            allocation_for<ValueType>& src_allocation
        ) noexcept(allocation_for<ValueType>::move_assignable_req >= expr_req::no_exception)
        {
            if constexpr(!always_equal_v)
                if(dst_alloc != src_alloc)
                    move_assign_impl(dst_alloc, dst_allocation, src_alloc, src_allocation);

            if constexpr(propagate_on_move_v) dst_alloc = cpp_move(src_alloc);
            dst_allocation = ::std::exchange(src_allocation, {});
        }

    private:
        template<typename ValueType>
        static constexpr void swap_impl(
            [[maybe_unused]] allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            [[maybe_unused]] allocator_type& src_alloc,
            allocation_for<ValueType>& src_allocation
        )
        {
            if constexpr(!always_equal_v && !propagate_on_swap_v)
                if(dst_alloc != src_alloc)
                {
                    if(src_allocation.has_value() && dst_allocation.has_value())
                    {
                        ::std::ranges::swap(dst_allocation.get(), src_allocation.get());
                        return;
                    }

                    constexpr auto impl = []( // clang-format off
                        allocator_type& dst_alloc,
                        allocation_for<ValueType>& dst_allocation,
                        allocator_type& src_alloc,
                        allocation_for<ValueType>& src_allocation
                    ) // clang-format on
                    {
                        src_allocation.construct(src_alloc, cpp_move(dst_allocation.get()));
                        dst_allocation.destroy(dst_alloc);
                    };

                    if(dst_allocation.has_value())
                        impl(dst_alloc, dst_allocation, src_alloc, src_allocation);
                    else if(src_allocation.has_value())
                        impl(src_alloc, src_allocation, dst_alloc, dst_allocation);

                    return;
                }

            ::std::swap(dst_allocation, src_allocation);
        }

    public:
        template<typename ValueType>
            requires(allocation_for<ValueType>::swappable_req >= expr_req::well_formed)
        static constexpr void swap(
            allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            allocator_type& src_alloc,
            allocation_for<ValueType>& src_allocation
        ) noexcept(allocation_for<ValueType>::swappable_req >= expr_req::no_exception)
        {
            swap_impl(dst_alloc, dst_allocation, src_alloc, src_allocation);
            if constexpr(propagate_on_swap_v) ::std::ranges::swap(dst_alloc, src_alloc);
        }
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

        constexpr void allocate(allocator_type& alloc, const size_type size) noexcept
        {
            if(size_ >= size) return;

            if(ptr_ != nullptr) deallocate(alloc);

            *this = make_allocation(size);
        }

        constexpr void deallocate(allocator_type& alloc) noexcept
        {
            traits::deallocate(alloc, ptr_, size_);
            *this = {};
        }
    };

    template<allocator_req Allocator>
    template<typename ValueType>
    class [[nodiscard]] allocator_aware_traits<Allocator>::allocation_for
    {
    public:
        using value_type = ValueType;

        static constexpr auto copy_constructible_req =
            traits::template construct_req<value_type, const value_type&>;

        static constexpr auto move_constructible_req = expr_req::no_exception;

        static constexpr auto copy_assignable_req = ::std::min(
            copy_constructible_req,
            get_expr_req(
                copy_assignable<value_type>,
                nothrow_copy_assignable<value_type> && (!propagate_on_copy_v || always_equal_v)
            )
        );

        static constexpr auto move_assignable_req = propagate_on_move_v ?
            expr_req::no_exception :
            ::std::min(
                traits::template construct_req<value_type, value_type>,
                get_expr_req(move_assignable<value_type>, nothrow_move_assignable<value_type>)
            );

        static constexpr auto swappable_req = propagate_on_swap_v || always_equal_v ? //
            expr_req::no_exception :
            get_expr_req(::std::swappable<value_type>, nothrow_swappable<value_type>);

        static constexpr special_mem_req mem_req{
            move_constructible_req,
            copy_constructible_req,
            move_assignable_req,
            copy_assignable_req,
            expr_req::no_exception,
            swappable_req //
        };

    private:
        allocation allocation_{};
        bool has_value_ = false;

    public:
        allocation_for() = default;

        constexpr allocation_for(const allocation& allocation, const bool has_value = false) //
            noexcept(!is_debug):
            allocation_(allocation), has_value_(has_value)
        {
            precondition<::std::invalid_argument>( //
                ::std::bind_front(
                    ::std::ranges::greater_equal{},
                    allocation_.size(),
                    sizeof(value_type)
                )
            );
        }

        [[nodiscard]] constexpr decltype(auto) get() const noexcept
        {
            return pointer_cast<value_type>(allocation_.begin());
        }

        [[nodiscard]] constexpr const auto& get_const() const noexcept { return get(); }

        [[nodiscard]] constexpr bool has_value() const noexcept { return has_value_; }

        constexpr void allocate(
            traits::allocator_type& alloc,
            const traits::const_void_pointer& hint = nullptr
        )
        {
            if(allocation_) return;

            allocation_ = make_allocation(alloc, sizeof(value_type), hint);
        }

        constexpr void deallocate(traits::allocator_type& alloc) noexcept
        {
            if(!allocation_) return;
            destroy(alloc);
            allocation_.deallocate(alloc);
        }

        template<
            typename... Args,
            auto Req = traits::template construct_req<value_type, Args...> // clang-format off
        > requires(Req >= expr_req::well_formed)
        [[nodiscard]] constexpr decltype(auto) construct(traits::allocator_type& alloc, Args&&... args)
            noexcept(Req >= expr_req::no_exception) // clang-format on
        {
            if(!allocation_) allocate(alloc);
            else destroy(alloc);

            decltype(auto) res = traits::construct(alloc, get(), cpp_forward(args)...);
            has_value_ = true;
            return res;
        }

        constexpr void destroy(traits::allocator_type& alloc) noexcept
        {
            if(!has_value()) return;
            traits::destroy(alloc, get());
            has_value_ = false;
        }
    };

    template<typename T>
        requires requires(const T t, T::allocator_type alloc) //
    {
        requires ::std::derived_from<T, allocator_aware_traits<decltype(alloc)>>;
        // clang-format off
        { t.get_allocator() } noexcept ->
            ::std::convertible_to<decltype(alloc)>; // clang-format on
    }
    struct allocator_aware_ctor : T
    {
        using this_t = allocator_aware_ctor;

        using T::T;
        using typename T::allocator_type;
        using allocator_traits = allocator_traits<allocator_type>;

    private:
        template<typename... Args>
        static constexpr auto alloc_last_ctor = constructible_from_test<T, Args..., allocator_type>;

        template<typename... Args>
        static constexpr auto alloc_arg_ctor =
            constructible_from_test<T, ::std::allocator_arg_t, allocator_type, Args...>;

    public:
        template<typename... Args>
            requires ::std::constructible_from<allocator_type> &&
            (alloc_last_ctor<Args...> >= expr_req::well_formed)
        constexpr allocator_aware_ctor(Args&&... args) //
            noexcept(alloc_last_ctor<Args...> >= expr_req::no_exception):
            T(cpp_forward(args)..., allocator_type{})
        {
        }

        template<typename... Args>
            requires ::std::constructible_from<allocator_type> &&
            (alloc_arg_ctor<Args...> >= expr_req::well_formed)
        constexpr allocator_aware_ctor(Args&&... args) //
            noexcept(alloc_arg_ctor<Args...> >= expr_req::no_exception):
            T(::std::allocator_arg, allocator_type{}, cpp_forward(args)...)
        {
        }

        template<typename... Args>
            requires ::std::constructible_from<T, Args..., allocator_type>
        constexpr allocator_aware_ctor(
            const ::std::allocator_arg_t,
            const allocator_type& alloc,
            Args&&... args
        ) noexcept(nothrow_constructible_from<T, Args..., allocator_type>):
            T(cpp_forward(args)..., alloc)
        {
        }

        constexpr allocator_aware_ctor(const this_t& other) //
            noexcept(nothrow_constructible_from<T, const T&, allocator_type>)
            requires ::std::constructible_from<T, const T&, allocator_type>
            :
            T( //
                static_cast<const T&>(other),
                allocator_traits::select_on_container_copy_construction(other.get_allocator())
            )
        {
        }

        constexpr allocator_aware_ctor(this_t&& other) //
            noexcept(nothrow_constructible_from<T, T, allocator_type>)
            requires ::std::constructible_from<T, T, allocator_type>
            : T(static_cast<T&&>(other), other.get_allocator())
        {
        }

        this_t& operator=(const this_t&) = default;
        this_t& operator=(this_t&&) = default; // NOLINT(*-noexcept-move-constructor)
        ~allocator_aware_ctor() = default;
    };
}
