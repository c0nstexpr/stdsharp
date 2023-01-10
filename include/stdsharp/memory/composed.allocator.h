#pragma once
#include <variant>

#include "allocator_traits.h"
#include "../default_operator.h"
#include "../functional/operations.h"

namespace stdsharp
{
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

        template<::std::size_t I>
        struct pointer_proxy
        {
            static constexpr ::std::variant<typename allocator_traits<Allocators>::pointer...>
                member_of_pointer(const pointer_variant& info)
            {
                return {::std::in_place_index<I>, ::std::get<I>(info)};
            }

            static constexpr auto post_increase(pointer_variant& info)
            {
                return post_increase_v(::std::get<I>(info));
            }

            static constexpr auto post_decrease(pointer_variant& info)
            {
                return post_decrease_v(::std::get<I>(info));
            }

            static constexpr auto indirection(const pointer_variant& info)
            {
                return subscript(info, 0);
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
                return {::std::in_place_index<I>, *(::std::get<I>(info).ptr + n)};
            }

            static constexpr auto
                not_equal(const pointer_variant& left, const pointer_variant& right)
            {
                return not_equal_to_v(::std::get<I>(left), ::std::get<I>(right));
            }

            static constexpr auto
                three_ways_compare(const pointer_variant& left, const pointer_variant& right)
            {
                return compare_three_way_v(::std::get<I>(left), ::std::get<I>(right));
            }

            static constexpr auto not_equal_to_nullptr(const pointer_variant& left, const nullptr_t)
            {
                return not_equal_to_v(::std::get<I>(left).ptr, nullptr);
            }

            static constexpr auto
                three_ways_compare_to_nullptr(const pointer_variant& left, const nullptr_t)
            {
                return compare_three_way_v(::std::get<I>(left).ptr, nullptr);
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

            alloc_info_variant info_;

            template<::std::size_t I, typename... Args>
                requires ::std::constructible_from<
                    alloc_info_variant,
                    ::std::in_place_index_t<I>,
                    Args...>
            constexpr pointer(
                const ::std::in_place_index_t<I> placeholder,
                Args&&... args
            ) noexcept(nothrow_constructible_from<alloc_info_variant, ::std::in_place_index_t<I>, Args...>):
                info_(placeholder, ::std::forward<Args>(args)...)
            {
            }

            friend ::std::ptrdiff_t operator-(const pointer& lhs, const pointer& rhs) noexcept
            {
                return lhs.ptr_ - rhs.ptr_;
            }

            template<::std::size_t I>
            struct deallocate_fn
            {
                static constexpr void invoke(composed_allocator& alloc, T* ptr) noexcept
                {
                    alloc_traits<I>::deallocate(::std::get<I>(alloc), ptr);
                }
            };

            constexpr void deallocate(composed_allocator& alloc) noexcept { deallocate_fn_(alloc); }

        public:
            using element_type = T;

            constexpr pointer(const nullptr_t = {}) {}
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