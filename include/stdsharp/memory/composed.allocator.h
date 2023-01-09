#pragma once

#include <type_traits>

#include "allocator_traits.h"
#include "../utility/value_wrapper.h"

namespace stdsharp
{
    template<typename T, allocator_req... Allocators>
        requires(::std::same_as<typename allocator_traits<Allocators>::value_type, T> && ...)
    class composed_allocator : ::std::tuple<Allocators...>
    {
        using base = ::std::tuple<Allocators...>;

        template<typename Alloc>
        using pointers_storage = ::std::shared_ptr<::std::vector<T*, Alloc>>;

        using pointers_tuple = ::std::tuple<pointers_storage<Allocators>...>;

        pointers_tuple pointers_;

        template<::std::size_t... I>
        constexpr void
            copy_assign_impl(const composed_allocator& other, const ::std::index_sequence<I...>)
        {
            auto f = [this, &other]<::std::size_t J>
            {
                if constexpr( //
                    allocator_traits<::std::tuple_element_t<J, ::std::tuple<Allocators...>>>::
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
                    allocator_traits<::std::tuple_element_t<J, ::std::tuple<Allocators...>>>::
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
            const void* hint = nullptr
        )
        {
            auto& alloc = ::std::get<I>(*this);

            try
            {
                return {alloc.allocate(count, hint), true};
            }
            catch(...)
            {
                return {nullptr, false};
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