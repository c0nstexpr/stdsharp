#pragma once
#include <variant>

#include "allocator_traits.h"
#include "../default_operator.h"
#include "../functional/operations.h"

namespace stdsharp
{
    template<::std::size_t I>
    class aggregate_bad_alloc : public ::std::bad_alloc
    {
        ::std::array<::std::exception_ptr, I> exceptions_;

    public:
        template<typename... Args>
        constexpr aggregate_bad_alloc(Args&&... args) noexcept:
            exceptions_{::std::forward<Args>(args)...}
        {
        }

        [[nodiscard]] constexpr const auto& exceptions() const noexcept { return exceptions_; }
    }

    template<typename T, allocator_req... Allocators>
        requires requires(
            allocator_traits<Allocators>... traits,
            typename decltype(traits)::pointer... pointers,
            typename decltype(traits)::const_pointer... const_pointers
        ) //
    {
        requires all_same<
            T,
            typename decltype(traits)::value_type...,
            ::std::iter_value_t<decltype(pointers)>... // clang-format off
        >; // clang-format on
        requires all_same<T&, ::std::iter_reference_t<decltype(pointers)>...>;
        requires all_same<const T&, ::std::iter_reference_t<decltype(const_pointers)>...>;

        typename ::std::common_type_t<typename decltype(traits)::difference_type...>;
        typename ::std::common_type_t<typename decltype(traits)::size_type...>;

        requires(::std::three_way_comparable<decltype(pointers), ::std::strong_ordering> && ...);
    }
    class composed_allocator : ::std::tuple<Allocators...>

    {
        using base = ::std::tuple<Allocators...>;

        template<::std::size_t I>
        using alloc_traits = allocator_traits<::std::tuple_element<I, base>>;

    public:
        using difference_type =
            ::std::common_type_t<typename allocator_traits<Allocators>::difference_type...>;

        using size_type = ::std::common_type_t<typename allocator_traits<Allocators>::size_type...>;

    private:
        using pointer_variant = ::std::variant<typename allocator_traits<Allocators>::pointer...>;

        using post_increase_fn = void (*)(pointer_variant&);
        using post_decrease_fn = void (*)(pointer_variant&);
        using plus_fn = T& (*)(pointer_variant&, difference_type);
        using minus_fn = T& (*)(pointer_variant&, difference_type);
        using subscript_fn = T& (*)(const pointer_variant&, difference_type);
        using three_ways_compare_fn =
            ::std::strong_ordering (*)(const pointer_variant&, const pointer_variant&);
        using equal_to_nullptr_fn = bool (*)(const pointer_variant&, const nullptr_t);
        using deallocate_fn = void (*)(base&, pointer_variant&, size_type) noexcept;

        template<::std::size_t I>
        struct pointer_proxy
        {
            static constexpr void post_increase(pointer_variant& info)
            {
                post_increase_v(::std::get<I>(info));
            }

            static constexpr void post_decrease(pointer_variant& info)
            {
                post_decrease_v(::std::get<I>(info));
            }

            static constexpr void plus(pointer_variant& info, const difference_type n)
            {
                plus_assign_v(::std::get<I>(info), n);
            }

            static constexpr void minus(pointer_variant& info, const difference_type n)
            {
                minus_assign_v(::std::get<I>(info), n);
            }

            static constexpr T& subscript(const pointer_variant& info, const difference_type n)
            {
                return *(::std::get<I>(info).ptr + n);
            }

            static constexpr ::std::strong_ordering
                three_ways_compare(const pointer_variant& left, const pointer_variant& right)
            {
                return compare_three_way_v(::std::get<I>(left), ::std::get<I>(right));
            }

            static constexpr bool equal_to_nullptr(const pointer_variant& left, const nullptr_t)
            {
                return equal_to_v(::std::get<I>(left).ptr, nullptr);
            }

            static constexpr void
                deallocate(base& tuple, const pointer_variant& info, const size_type n) noexcept
            {
                alloc_traits<I>::deallocate(::std::get<I>(tuple), ::std::get<I>(info), n);
            }
        };

    public:
        class pointer;

        class pointer :
            default_post_increase_and_decrease<pointer>,
            default_arithmetic_operation<pointer>
        {
            friend class composed_allocator;
            friend class pointer_delegate;

            static constexpr auto operator_impl_ =
                []<::std::size_t... I>(const ::std::index_sequence<I...>)
            {
                return ::std::tuple<
                    ::std::array<post_increase_fn, sizeof...(I)>,
                    ::std::array<post_decrease_fn, sizeof...(I)>,
                    ::std::array<plus_fn, sizeof...(I)>,
                    ::std::array<minus_fn, sizeof...(I)>,
                    ::std::array<subscript_fn, sizeof...(I)>,
                    ::std::array<three_ways_compare_fn, sizeof...(I)>,
                    ::std::array<equal_to_nullptr_fn, sizeof...(I)>,
                    ::std::array<deallocate_fn, sizeof...(I)> // clang-format off
                >{
                    {&pointer_proxy<I>::post_increase...}, // clang-format on
                    {&pointer_proxy<I>::post_decrease...},
                    {&pointer_proxy<I>::plus...},
                    {&pointer_proxy<I>::minus...},
                    {&pointer_proxy<I>::subscript...},
                    {&pointer_proxy<I>::three_ways_compare...},
                    {&pointer_proxy<I>::equal_to_nullptr...},
                    {&pointer_proxy<I>::deallocate...} //
                };
            }
            (::std::make_index_sequence<sizeof...(Allocators)>());

            static constexpr auto post_increase_impl = ::std::get<0>(operator_impl_);
            static constexpr auto post_decrease_impl = ::std::get<1>(operator_impl_);
            static constexpr auto plus_impl = ::std::get<2>(operator_impl_);
            static constexpr auto minus_impl = ::std::get<3>(operator_impl_);
            static constexpr auto subscript_impl = ::std::get<4>(operator_impl_);
            static constexpr auto three_ways_compare_impl = ::std::get<5>(operator_impl_);
            static constexpr auto equal_to_nullptr_impl = ::std::get<6>(operator_impl_);
            static constexpr auto deallocate_impl = ::std::get<7>(operator_impl_);

            pointer_variant ptr_;

            template<::std::size_t I, typename... Args>
                requires ::std::constructible_from<
                    pointer_variant,
                    ::std::in_place_index_t<I>,
                    Args...>
            constexpr pointer(
                const ::std::in_place_index_t<I> placeholder,
                Args&&... args
            ) noexcept(nothrow_constructible_from<pointer_variant, ::std::in_place_index_t<I>, Args...>):
                ptr_(placeholder, ::std::forward<Args>(args)...)
            {
            }

            constexpr void deallocate(base& tuple, const size_type n) noexcept
            {
                deallocate_impl[ptr_.index()](tuple, ptr_, n);
            }

        public:
            using element_type = T;

            pointer() = default;

            constexpr pointer(const nullptr_t) noexcept {}

            constexpr const pointer operator++(int) // NOLINT(*-const-return-type)
            {
                const auto result = *this;
                post_increase_impl[ptr_.index()](ptr_);
                return result;
            }

            constexpr const pointer operator--(int) // NOLINT(*-const-return-type)
            {
                const auto result = *this;
                post_decrease_impl[ptr_.index()](ptr_);
                return result;
            }

            constexpr pointer& operator+(const difference_type n)
            {
                plus_impl[ptr_.index()](ptr_, n);
                return *this;
            }

            constexpr pointer& operator-(const difference_type n)
            {
                minus_impl[ptr_.index()](ptr_, n);
                return *this;
            }

            constexpr T& operator[](const difference_type n)
            {
                return subscript_impl[ptr_.index()](ptr_, n);
            }

            constexpr ::std::strong_ordering operator<=>(const pointer& right) const
            {
                return three_ways_compare_impl[ptr_.index()](ptr_, right.ptr_);
            }

            constexpr bool operator!=(const pointer& right) const { return (*this <=> right) != 0; }

            constexpr bool operator==(const nullptr_t) const
            {
                return equal_to_nullptr_impl[ptr_.index()](ptr_, nullptr);
            }

            constexpr bool operator!=(const nullptr_t) const { return !(*this == nullptr); }

            friend constexpr bool operator==(const nullptr_t, const pointer& right)
            {
                return right == nullptr;
            }

            friend constexpr bool operator!=(const nullptr_t, const pointer& right)
            {
                return right != nullptr;
            }

            constexpr T& operator*() const { return (*this)[0]; }

            constexpr T* operator->() const { return ::std::addressof(**this); }

            constexpr explicit operator bool() const { return *this != nullptr; }

            constexpr bool operator!() const { return !static_cast<bool>(*this); }
        };

    private:
        template<::std::size_t... I>
        constexpr void
            copy_assign_impl(const composed_allocator& other, const ::std::index_sequence<I...>)
        {
            auto f = [this, &other]<::std::size_t J>
            {
                if constexpr( //
                    allocator_traits<::std::tuple_element_t<J, base>>::
                        propagate_on_container_copy_assignment::value //
                )
                    ::std::get<J>(*this) = ::std::get<J>(other);
            };

            (f(index_constant<I>{}), ...);
        }

        template<::std::size_t... I>
        constexpr void
            move_assign_impl(composed_allocator&& other, const ::std::index_sequence<I...>) noexcept
        {
            auto f = [this, &other]<::std::size_t J>
            {
                if constexpr( //
                    allocator_traits<::std::tuple_element_t<J, base>>::
                        propagate_on_container_move_assignment::value //
                )
                    ::std::get<J>(*this) = ::std::move(::std::get<J>(other));
            };

            (f(index_constant<I>{}), ...);
        }

        template<::std::size_t I>
        [[nodiscard]] constexpr ::std::pair<T*, bool> allocate_impl_by(
            const index_constant<I>,
            const ::std::size_t count,
            const void* hint,
            T*& out_ptr
        )
        {
            auto& alloc = ::std::get<I>(*this);

            try
            {
                out_ptr = alloc.allocate(count, hint);
                return true;
            }
            catch(...)
            {
                return false;
            }
        }

        template<::std::size_t... I>
        [[nodiscard]] constexpr T* allocate_impl(
            const ::std::index_sequence<I...>,
            const ::std::size_t count,
            const void* hint = nullptr
        )
        {
            const auto allocate_f = [this, count, hint]<::std::size_t J>(const index_constant<J>)
            {
                return allocate_impl_by(index_constant<J>{}, count, hint); //
            };
        }

    public:
        composed_allocator() = default;
        ~composed_allocator() = default;

        using base::base;

        composed_allocator(const composed_allocator&) = default;
        composed_allocator(composed_allocator&&) noexcept = default;

        using value_type = T;

        using propagate_on_container_copy_assignment = ::std::true_type;
        using propagate_on_container_move_assignment = ::std::true_type;
        using propagate_on_container_swap = ::std::true_type;
        using is_always_equal = ::std::false_type;

        constexpr composed_allocator& operator=(const composed_allocator& other)
        {
            copy_assign_impl(other, ::std::index_sequence_for<Allocators...>{});
            return *this;
        }

        constexpr composed_allocator& operator=(composed_allocator&& other) noexcept
        {
            move_assign_impl(::std::move(other), ::std::index_sequence_for<Allocators...>{});
            return *this;
        }

        // allocate the memory in sequence of allocators
        [[nodiscard]] constexpr T* allocate(const ::std::size_t count, const void* hint = nullptr)
        {
            return allocate_impl(::std::index_sequence_for<Allocators...>{}, count, hint);
        }

        constexpr void deallocate(T* ptr, const ::std::size_t count) noexcept {}

        [[nodiscard]] constexpr bool operator==(const composed_allocator&) const noexcept
        {
            return false;
        }

        [[nodiscard]] constexpr auto max_size() const noexcept {}
    };

}