#pragma once

#include <memory>
#include <iterator>

#include "../concepts/concepts.h"
#include "../type_traits/core_traits.h"

namespace stdsharp
{
    using max_alignment = ::std::alignment_of<::std::max_align_t>;

    inline constexpr auto max_alignment_v = max_alignment::value;

    namespace details
    {
        struct alloc_req_dummy_t
        {
            int v;
        };

        template<typename T>
        concept allocator_pointer =
            nullable_pointer<T> && nothrow_default_initializable<T> && nothrow_movable<T> &&
            nothrow_swappable<T> && nothrow_copyable<T> && nothrow_weakly_equality_comparable<T> &&
            nothrow_weakly_equality_comparable_with<T, ::std::nullptr_t>;
    }

    template<typename T>
    concept allocator_req = requires //
    {
        typename T::value_type;
        nothrow_copyable<T>;
        nothrow_movable<T>;

        requires requires(
            T alloc,
            ::std::allocator_traits<T> t_traits,
            typename decltype(t_traits)::pointer p,
            typename decltype(t_traits)::const_pointer const_p,
            typename decltype(t_traits)::void_pointer void_p,
            typename decltype(t_traits)::const_void_pointer const_void_p,
            typename decltype(t_traits)::value_type v,
            typename decltype(t_traits)::size_type size,
            typename decltype(t_traits):: //
            template rebind_traits<details::alloc_req_dummy_t> u_traits,
            typename decltype(t_traits)::is_always_equal always_equal,
            typename decltype(t_traits)::propagate_on_container_copy_assignment copy_assign,
            typename decltype(t_traits)::propagate_on_container_move_assignment move_assign,
            typename decltype(t_traits)::propagate_on_container_swap swap // clang-format off
        ) // clang-format on
        {
            requires !const_volatile<decltype(v)>;

            requires details::allocator_pointer<decltype(p)> &&
                details::allocator_pointer<decltype(const_p)> &&
                details::allocator_pointer<decltype(void_p)> &&
                details::allocator_pointer<decltype(const_void_p)>;

            requires ::std::random_access_iterator<decltype(p)> &&
                ::std::contiguous_iterator<decltype(p)>;

            requires nothrow_convertible_to<decltype(p), decltype(const_p)> &&
                ::std::random_access_iterator<decltype(const_p)> &&
                ::std::contiguous_iterator<decltype(const_p)>;

            requires nothrow_convertible_to<decltype(p), decltype(void_p)> &&
                nothrow_explicitly_convertible<decltype(void_p), decltype(p)> &&
                ::std::same_as<decltype(void_p), typename decltype(u_traits)::void_pointer>;

            requires nothrow_convertible_to<decltype(p), decltype(const_void_p)> &&
                nothrow_convertible_to<decltype(const_p), decltype(const_void_p)> &&
                nothrow_explicitly_convertible<decltype(const_void_p), decltype(const_p)> &&
                nothrow_convertible_to<decltype(void_p), decltype(const_void_p)> &&
                ::std::same_as< // clang-format off
                    decltype(const_void_p),
                    typename decltype(u_traits)::const_void_pointer
                >; // clang-format on

            requires ::std::unsigned_integral<decltype(size)>;

            requires ::std::signed_integral<typename decltype(t_traits)::difference_type>;

            requires ::std::
                same_as<typename decltype(u_traits)::template rebind_alloc<decltype(v)>, T>;
            // clang-format off

            { *p } -> ::std::same_as<::std::add_lvalue_reference_t<decltype(v)>>;
            { *const_p } -> ::std::same_as<add_const_lvalue_ref_t<decltype(v)>>;
            { *const_p } -> ::std::same_as<add_const_lvalue_ref_t<decltype(v)>>; // clang-format on

            requires requires(
                typename decltype(u_traits)::pointer other_p,
                typename decltype(u_traits)::const_pointer other_const_p,
                typename decltype(u_traits)::allocator_type u_alloc,
                typename decltype(t_traits)::allocator_type another_alloc // clang-format off
            )
            {
                nothrow_constructible_from<T, const decltype(u_alloc)&>;
                nothrow_constructible_from<T, decltype(u_alloc)>;

                { other_p->v } -> ::std::same_as<decltype(((*other_p).v))>;
                { other_const_p->v } -> ::std::same_as<decltype(((*other_const_p).v))>;

                t_traits.construct(alloc, other_p);
                t_traits.destroy(alloc, other_p);

                requires noexcept(alloc == another_alloc) && noexcept(alloc != another_alloc);
            }; // clang-format on

            ::std::pointer_traits<decltype(p)>::pointer_to(v); // clang-format off

            { t_traits.allocate(alloc, size) } -> ::std::same_as<decltype(p)>;
            { t_traits.allocate(alloc, size, const_void_p) } -> ::std::same_as<decltype(p)>;
            // { t_traits.allocate_at_least(alloc, size) } -> ::std::same_as<::std::allocation_result<decltype(p)>>;
            noexcept(alloc.deallocate(p, size));
            { t_traits.max_size(alloc) } -> ::std::same_as<decltype(size)>;
            // clang-format on

            requires ::std::derived_from<decltype(always_equal), ::std::true_type> ||
                ::std::derived_from<decltype(always_equal), ::std::false_type>;

            // clang-format off
            { t_traits.select_on_container_copy_construction(alloc) } -> ::std::same_as<T>;
            // clang-format on

            requires ::std::derived_from<decltype(copy_assign), ::std::true_type> &&
                    nothrow_copy_assignable<T> ||
                    ::std::derived_from<decltype(copy_assign), ::std::false_type>;

            requires ::std::derived_from<decltype(move_assign), ::std::true_type> &&
                    nothrow_move_assignable<T> ||
                    ::std::derived_from<decltype(move_assign), ::std::false_type>;

            requires ::std::derived_from<decltype(swap), ::std::true_type> &&
                    nothrow_swappable<T> || ::std::derived_from<decltype(swap), ::std::false_type>;
        };
    };

    template<allocator_req T>
    struct allocator_traits : private ::std::allocator_traits<T>
    {
    private:
        using base = ::std::allocator_traits<T>;

    public:
        using typename base::allocator_type;
        using typename base::value_type;
        using typename base::pointer;
        using typename base::const_pointer;
        using typename base::void_pointer;
        using typename base::const_void_pointer;
        using typename base::difference_type;
        using typename base::size_type;
        using typename base::propagate_on_container_copy_assignment;
        using typename base::propagate_on_container_move_assignment;
        using typename base::propagate_on_container_swap;
        using typename base::is_always_equal;

        template<typename U>
        using rebind_alloc = typename base::template rebind_alloc<U>;

        template<typename U>
        using rebind_traits = typename base::template rebind_traits<U>;

        using base::deallocate;
        using base::max_size;
        using base::select_on_container_copy_construction;

        static constexpr pointer allocate(
            allocator_type& alloc,
            const size_type count,
            const const_void_pointer hint = nullptr
        )
        {
            return hint == nullptr ? base::allocate(alloc, count) :
                                     base::allocate(alloc, count, hint);
        }

        template<typename U, typename... Args>
            requires ::std::constructible_from<U, Args...>
        static constexpr void construct(T& a, U* const ptr, Args&&... args)
        {
            base::construct(a, ptr, ::std::forward<Args>(args)...);
        }

        template<typename U>
            requires ::std::destructible<U>
        static constexpr void destroy(T& a, U* const ptr)
        {
            base::destroy(a, ptr);
        }

        static constexpr pointer try_allocate(
            allocator_type& alloc,
            const size_type count,
            const const_void_pointer hint = nullptr
        ) noexcept
        {
            if(max_size(alloc) < count) return nullptr;

            try
            {
                return allocate(alloc, count, hint);
            }
            catch(...)
            {
                return nullptr;
            }
        }

        static constexpr pointer try_allocate(
            allocator_type& alloc,
            const size_type count,
            const const_void_pointer hint = nullptr
        ) noexcept
            requires requires // clang-format off
        {
            { alloc.try_allocate(count, hint) } ->
                ::std::same_as<pointer>;
            requires noexcept(alloc.try_allocate(count, hint));
            { alloc.try_allocate(count) } ->
                ::std::same_as<pointer>; // clang-format on
            requires noexcept(alloc.try_allocate(count));
        }
        {
            return hint == nullptr ? alloc.try_allocate(count) : alloc.try_allocate(count, hint);
        }

        static constexpr auto allocate_at_least(allocator_type& alloc, const size_type count)
        {
            return ::std::allocate_at_least(alloc, count);
        }
    };


    template<typename>
    struct allocator_of;

    template<typename T>
    using allocator_of_t = ::meta::_t<allocator_of<T>>;
}