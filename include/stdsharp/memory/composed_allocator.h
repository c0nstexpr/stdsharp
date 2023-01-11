#pragma once
#include <exception>
#include <variant>

#include "allocator_traits.h"
#include "../default_operator.h"
#include "../functional/operations.h"
#include "../type_traits/function.h"

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
    };

    template<::std::size_t I>
    aggregate_bad_alloc(const ::std::array<::std::exception_ptr, I>&) -> aggregate_bad_alloc<I>;

    template<::std::size_t I>
    aggregate_bad_alloc(::std::array<::std::exception_ptr, I>&&) -> aggregate_bad_alloc<I>;

    namespace details
    {
        template<typename... T>
        concept common_and_inter_convertible = requires //
        {
            typename ::std::common_type_t<T...>;
            requires(::std::convertible_to<::std::common_type_t<T...>, T> && ...);
        };
    }

    template<typename T, allocator_req... Allocators>
        requires requires(
            allocator_traits<Allocators>... traits,
            typename decltype(traits)::pointer... pointers,
            typename decltype(traits)::const_pointer... const_pointers
        ) //
    {
        requires details::common_and_inter_convertible<typename decltype(traits)::pointer...>;
        typename ::std::common_type_t<typename decltype(traits)::const_pointer...>;
        requires details::common_and_inter_convertible< // clang-format off
            typename decltype(traits)::const_void_pointer...
        >; // clang-format on
        requires details::common_and_inter_convertible< // clang-format off
            typename decltype(traits)::difference_type...
        >; // clang-format on
        requires details::common_and_inter_convertible<typename decltype(traits)::size_type...>;
    }
    class composed_allocator : indexed_values<Allocators...>
    {
        using base = indexed_values<Allocators...>;

        template<::std::size_t I>
        using alloc_t = typename base::template type<I>;

        template<::std::size_t I>
        using alloc_traits = allocator_traits<alloc_t<I>>;

        template<::std::size_t I>
        using alloc_pointer = typename alloc_traits<I>::pointer;

        template<::std::size_t I>
        using alloc_const_void_pointer = typename alloc_traits<I>::const_void_pointer;

        template<::std::size_t I>
        using alloc_difference_type = typename alloc_traits<I>::difference_type;

        template<::std::size_t I>
        using alloc_size_type = typename alloc_traits<I>::size_type;

    public:
        using value_type = T;

        using difference_type =
            ::std::common_type_t<typename allocator_traits<Allocators>::difference_type...>;

        using size_type = ::std::common_type_t<typename allocator_traits<Allocators>::size_type...>;

        using pointer = ::std::common_type_t<typename allocator_traits<Allocators>::pointer...>;

        using const_pointer =
            ::std::common_type_t<typename allocator_traits<Allocators>::const_pointer...>;

        using const_void_pointer =
            ::std::common_type_t<typename allocator_traits<Allocators>::const_void_pointer...>;

        using propagate_on_container_copy_assignment = ::std::disjunction<
            typename allocator_traits<Allocators>::propagate_on_container_copy_assignment...>;

        using propagate_on_container_move_assignment = ::std::disjunction<
            typename allocator_traits<Allocators>::propagate_on_container_move_assignment...>;

        using propagate_on_container_swap = ::std::disjunction<
            typename allocator_traits<Allocators>::propagate_on_container_swap...>;

        using is_always_equal =
            ::std::conjunction<typename allocator_traits<Allocators>::is_always_equal...>;

        template<typename U>
        struct rebind
        {
            using other = composed_allocator<
                U,
                typename allocator_traits<Allocators>::template rebind_alloc<U>...>;
        };

    private:
        template<::std::size_t... I>
        constexpr void
            copy_assign_impl(const composed_allocator& other, const ::std::index_sequence<I...>) noexcept(
                (nothrow_copy_assignable<Allocators> && ...)
            )
        {
            auto f = [this, &other]<::std::size_t J>
            {
                if constexpr( //
                    allocator_traits<alloc_t<J>>::propagate_on_container_copy_assignment::value //
                )
                    get<J>(*this) = get<J>(other);
            };

            (f(index_constant<I>{}), ...);
        }

        template<::std::size_t... I>
        constexpr void
            move_assign_impl(composed_allocator&& other, const ::std::index_sequence<I...>) noexcept(
                (nothrow_move_assignable<Allocators> && ...)
            )
        {
            auto f = [this, &other]<::std::size_t J>
            {
                if constexpr( //
                    allocator_traits<alloc_t<J>>::propagate_on_container_move_assignment::value //
                )
                    get<J>(*this) = ::std::move(get<J>(other));
            };

            (f(index_constant<I>{}), ...);
        }

        template<::std::size_t... I>
        constexpr void
            swap_impl(composed_allocator& other, const ::std::index_sequence<I...>) noexcept(
                (nothrow_swappable<Allocators> && ...)
            )
        {
            auto f = [this, &other]<::std::size_t J>
            {
                if constexpr( //
                    allocator_traits<alloc_t<J>>::propagate_on_container_swap::value //
                )
                    ::std::ranges::swap(get<J>(*this), get<J>(other));
            };

            (f(index_constant<I>{}), ...);
        }

        template<bool Noexcept, ::std::size_t I>
        constexpr ::std::variant<
            pointer,
            ::std::conditional_t<Noexcept, empty_t, ::std::exception_ptr> // clang-format off
        > allocate_at(const alloc_size_type<I> count, const alloc_const_void_pointer<I>& hint)
            noexcept // clang-format on
        {
            try
            {
                return {
                    ::std::in_place_index<0>,
                    static_cast<pointer>(alloc_traits<I>::allocate(get<I>(*this), count, hint)) //
                };
            }
            catch(...)
            {
                if constexpr(Noexcept) return {::std::in_place_index<1>};
                else return {::std::in_place_index<1>, ::std::current_exception()};
            }
        }

        template<::std::size_t I>
            requires requires(
                alloc_t<I> alloc,
                alloc_size_type<I> count,
                alloc_const_void_pointer<I> hint
            ) // clang-format off
        {
            { alloc.try_allocate(count, hint) } ->
                ::std::convertible_to<pointer>; // clang-format on
            requires noexcept(alloc.try_allocate(count, hint));
        }
        constexpr pointer try_allocate_at(
            const alloc_size_type<I> count,
            const alloc_const_void_pointer<I>& hint
        ) noexcept
        {
            try
            {
                return auto_cast(get<I>(*this).try_allocate(count, hint));
            }
            catch(...)
            {
                return nullptr;
            }
        }

        template<bool Noexcept, ::std::size_t... I>
        constexpr pointer allocate_impl(
            const ::std::index_sequence<I...>,
            const size_type count,
            const const_void_pointer& hint
        ) noexcept(Noexcept)
        {
            pointer ptr{};
            ::std::conditional_t<
                Noexcept,
                empty_t,
                ::std::array<::std::exception_ptr, sizeof...(I)> // clang-format off
            > exception{}; // clang-format on

            const auto allocate_f = [this, &ptr, &exception]<::std::size_t J>( // clang-format off
                const index_constant<J>,
                const size_type count,
                const_void_pointer hint
            ) noexcept(Noexcept) // clang-format on
            {
                const alloc_size_type<J> count_v = auto_cast(count);

                try
                {
                    const alloc_const_void_pointer<J> hint_v = auto_cast(::std::move(hint));

                    if constexpr(requires { try_allocate<J>(count_v, hint_v); })
                    {
                        auto&& res_ptr = try_allocate<J>(count_v, hint_v);

                        if(res_ptr != nullptr)
                        {
                            ptr = ::std::move(res_ptr);
                            return true;
                        }
                    }
                    else
                    {
                        auto&& res_ptr = allocate_at<Noexcept, J>(count_v, hint_v);

                        if(res_ptr.index() == 0)
                        {
                            ptr = ::std::move(::std::get<0>(res_ptr));
                            return true;
                        }

                        if constexpr(!Noexcept) exception[J] = ::std::move(::std::get<1>(res_ptr));

                        return false;
                    }
                }
                catch(...)
                {
                    if constexpr(!Noexcept) exception[J] = ::std::current_exception();
                    return false;
                }
            };

            if(!(allocate_f(index_constant<I>{}, count, hint) || ...))
                if constexpr(!Noexcept) throw aggregate_bad_alloc{::std::move(exception)};

            return ptr;
        }

        template<::std::size_t... I>
        constexpr bool
            equal_impl(const ::std::index_sequence<I...>, const composed_allocator& other)
                const noexcept
        {
            return ((get<I>(*this) == get<I>(other)) && ...);
        }

        template<::std::size_t... I>
            requires is_always_equal::value
        constexpr bool
            equal_impl(const ::std::index_sequence<I...>, const composed_allocator&) const noexcept
        {
            return true;
        }

        friend constexpr bool
            operator==(const composed_allocator& left, const composed_allocator& right) noexcept
        {
            return left.equal_impl(::std::index_sequence_for<Allocators...>{}, right);
        };

    public:
        using base::base;

        composed_allocator() = default;
        ~composed_allocator() = default;

        composed_allocator(const composed_allocator&) = default;
        composed_allocator(composed_allocator&&) noexcept = default;

        constexpr composed_allocator& operator=(const composed_allocator& other) noexcept( //
            noexcept(copy_assign_impl(other, ::std::index_sequence_for<Allocators...>{}))
        )
            requires(propagate_on_container_copy_assignment::value)
        {
            copy_assign_impl(other, ::std::index_sequence_for<Allocators...>{});
            return *this;
        }

        composed_allocator& operator=(const composed_allocator& other) = default;

        constexpr composed_allocator& operator=(composed_allocator&& other) noexcept( //
            noexcept(
                move_assign_impl(::std::move(other), ::std::index_sequence_for<Allocators...>{})
            )
        )
            requires(propagate_on_container_move_assignment::value)
        {
            move_assign_impl(::std::move(other), ::std::index_sequence_for<Allocators...>{});
            return *this;
        }

        composed_allocator& operator=(composed_allocator&& other) noexcept = default;

        constexpr void swap(composed_allocator& other) //
            noexcept(noexcept(swap_impl(other, ::std::index_sequence_for<Allocators...>{})))
            requires(propagate_on_container_swap::value)
        {
            swap_impl(other, ::std::index_sequence_for<Allocators...>{});
        }

        constexpr void swap(composed_allocator& other) noexcept(nothrow_swappable<base>)
        {
            ::std::ranges::swap(static_cast<base&>(*this), static_cast<base&>(other));
        }

        // allocate the memory in sequence of allocators
        [[nodiscard]] constexpr pointer
            allocate(const size_type count, const const_void_pointer& hint = nullptr)
        {
            return allocate_impl<false>(::std::index_sequence_for<Allocators...>{}, count, hint);
        }

        // allocate the memory in sequence of allocators
        [[nodiscard]] constexpr pointer
            try_allocate(const size_type count, const const_void_pointer& hint = nullptr) noexcept
        {
            return allocate_impl<true>(::std::index_sequence_for<Allocators...>{}, count, hint);
        }

        constexpr void deallocate(pointer& ptr, const size_type count) noexcept
        {
            ptr.deallocate(static_cast<base&>(*this), count);
        }

        [[nodiscard]] constexpr auto max_size() const noexcept {}
    };
}