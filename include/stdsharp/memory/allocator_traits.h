#pragma once

#include <iterator>
#include <memory>

#include "../functional/bind.h"
#include "../type_traits/special_member.h"

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
    } // namespace details

    template<typename Alloc>
    concept allocator_req = requires //
    {
        typename Alloc::value_type;
        nothrow_copyable<Alloc>;

        requires requires(
            Alloc alloc,
            ::std::allocator_traits<Alloc> t_traits,
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
                nothrow_convertible_to<
                         decltype(void_p),
                         decltype(const_void_p)> &&
                ::std::same_as< // clang-format off
                    decltype(const_void_p),
                    typename decltype(u_traits)::const_void_pointer
                >; // clang-format on

            requires ::std::unsigned_integral<decltype(size)>;

            requires ::std::signed_integral<typename decltype(t_traits)::difference_type>;

            requires ::std::
                same_as<typename decltype(u_traits)::template rebind_alloc<decltype(v)>, Alloc>;
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
                nothrow_constructible_from<Alloc, const decltype(u_alloc)&>;
                nothrow_constructible_from<Alloc, decltype(u_alloc)>;

                { other_p->v } -> ::std::same_as<decltype(((*other_p).v))>;
                { other_const_p->v } -> ::std::same_as<decltype(((*other_const_p).v))>;

                t_traits.construct(alloc, other_p);
                t_traits.destroy(alloc, other_p);

                requires noexcept(alloc == another_alloc) && noexcept(alloc != another_alloc);
            }; // clang-format on

            ::std::pointer_traits<decltype(p)>::pointer_to(v); // clang-format off

            { t_traits.allocate(alloc, size) } -> ::std::same_as<decltype(p)>;
            { t_traits.allocate(alloc, size, const_void_p) } -> ::std::same_as<decltype(p)>;
            noexcept(alloc.deallocate(p, size));
            { t_traits.max_size(alloc) } -> ::std::same_as<decltype(size)>;
            // clang-format on

            requires ::std::derived_from<decltype(always_equal), ::std::true_type> ||
                ::std::derived_from<decltype(always_equal), ::std::false_type>;

            // clang-format off
            { t_traits.select_on_container_copy_construction(alloc) } -> ::std::same_as<Alloc>;
            // clang-format on

            requires ::std::derived_from<decltype(copy_assign), ::std::true_type> &&
                    nothrow_copy_assignable<Alloc> ||
                    ::std::derived_from<decltype(copy_assign), ::std::false_type>;

            requires ::std::derived_from<decltype(move_assign), ::std::true_type> &&
                    nothrow_move_assignable<Alloc> ||
                    ::std::derived_from<decltype(move_assign), ::std::false_type>;

            requires ::std::derived_from<decltype(swap), ::std::true_type> &&
                    nothrow_swappable<Alloc> ||
                    ::std::derived_from<decltype(swap), ::std::false_type>;
        };
    };

    template<typename T>
    struct make_obj_uses_allocator_fn
    {
    private:
        static constexpr struct using_allocator_fn
        {
            template<typename Alloc, typename... Args>
                requires ::std::constructible_from<T, Args..., const Alloc&>
            constexpr T operator()(const Alloc& alloc, Args&&... args) const
                noexcept(nothrow_constructible_from<T, Args..., const Alloc&>)
            {
                return T{::std::forward<Args>(args)..., alloc};
            }

            template<typename Alloc, typename... Args>
                requires ::std::constructible_from<T, ::std::allocator_arg_t, const Alloc&, Args...>
            constexpr T operator()(const Alloc& alloc, Args&&... args) const
                noexcept(nothrow_constructible_from<
                         T,
                         ::std::allocator_arg_t,
                         const Alloc&,
                         Args...>)
            {
                return T{::std::allocator_arg, alloc, ::std::forward<Args>(args)...};
            }
        } using_allocator{};

    public:
        template<typename Alloc, typename... Args>
            requires ::std::uses_allocator_v<T, Alloc>
        constexpr T operator()(const Alloc& alloc, Args&&... args) const
            noexcept(nothrow_invocable<using_allocator_fn, const Alloc&, Args...>)
        {
            return using_allocator(alloc, ::std::forward<Args>(args)...);
        }

        template<typename Alloc, typename... Args>
            requires ::std::constructible_from<T, Args...> && (!::std::uses_allocator<T, Alloc>{})
        constexpr T operator()(const Alloc&, Args&&... args) const
            noexcept(nothrow_constructible_from<T, Args...>)

        {
            return T{::std::forward<Args>(args)...};
        }
    };

    template<typename T>
    inline constexpr make_obj_uses_allocator_fn<T> make_obj_uses_allocator{};

    enum class allocator_assign_operation
    {
        before_assign,
        after_assign
    };

    enum class allocator_swap_operation
    {
        before_swap,
        after_swap
    };

    template<allocator_req Alloc>
    struct allocator_traits : private ::std::allocator_traits<Alloc>
    {
        static constexpr struct construct_fn
        {
            template<typename U, typename... Args>
            static constexpr auto alloc_custom_construct =
                requires(Alloc & a, U* const ptr, Args&&... args) {
                    a.construct(ptr, ::std::declval<Args>()...);
                };

            template<typename U, typename... Args>
                requires alloc_custom_construct<U, Args...>
            constexpr decltype(auto) operator()(Alloc& a, U* const ptr, Args&&... args) const
                noexcept(noexcept(a.construct(ptr, ::std::declval<Args>()...)))
            {
                return a.construct(ptr, ::std::forward<Args>(args)...);
            }

            template<typename... Args, uses_allocator_constructible<Alloc, Args...> U>
                requires(!alloc_custom_construct<U, Args...>)
            constexpr void operator()(Alloc& a, U* const ptr, Args&&... args) const
                noexcept(nothrow_uses_allocator_constructible<U, Alloc, Args...>)
            {
                if constexpr(::std::is_constructible_v<U, Args..., const Alloc&>)
                    ::std::construct_at(ptr, ::std::forward<Args>(args)..., a);
                else
                    ::std::construct_at(
                        ptr,
                        ::std::allocator_arg,
                        a,
                        ::std::forward<Args>(args)...
                    );
            }

            template<typename... Args, typename U>
                requires(!(alloc_custom_construct<U, Args...> ||
                           uses_allocator_constructible<U, Alloc, Args...>)) &&
                ::std::is_constructible_v<U, Args...>
            constexpr void operator()(Alloc&, U* const ptr, Args&&... args) const
                noexcept(stdsharp::nothrow_constructible_from<U, Args...>)
            {
                ::std::construct_at(ptr, ::std::forward<Args>(args)...);
            }
        } construct{};

        template<typename T, typename... Args>
        static constexpr auto construct_req = ::std::invocable<construct_fn, Alloc&, T*, Args...> ?
            nothrow_invocable<construct_fn, Alloc&, T*, Args...> ? //
                expr_req::no_exception :
                expr_req::well_formed :
            expr_req::ill_formed;

        static constexpr struct destroy_fn
        {
            template<typename U>
            static constexpr auto alloc_custom_destroy =
                requires(Alloc & a, U* const ptr) { a.destroy(ptr); };

            template<typename U>
                requires alloc_custom_destroy<U>
            constexpr void operator()(Alloc& a, U* const ptr) const
                noexcept(noexcept(a.destroy(ptr)))
            {
                a.destroy(ptr);
            }

            template<typename U>
                requires(::std::is_destructible_v<U> && !alloc_custom_destroy<U>)
            constexpr void operator()(Alloc&, U* const ptr) const noexcept(::std::destructible<U>)
            {
                ::std::destroy_at(ptr);
            }
        } destroy{};

        template<typename T>
        static constexpr auto destroy_req = ::std::invocable<destroy_fn, Alloc&, T*> ?
            nothrow_invocable<destroy_fn, Alloc&, T*> ? //
                expr_req::no_exception :
                expr_req::well_formed :
            expr_req::ill_formed;

    private:
        using base = ::std::allocator_traits<Alloc>;

        template<typename T, typename... Args>
        static constexpr auto constructible_from =
            ::std::invocable<construct_fn, Alloc&, T*, Args...>;

        template<typename T, typename... Args>
        static constexpr auto nothrow_constructible_from =
            nothrow_invocable<construct_fn, Alloc&, T*, Args...>;

        template<typename T>
        static constexpr auto destructible = ::std::invocable<destroy_fn, Alloc&, T*>;

        template<typename T>
        static constexpr auto nothrow_destructible = nothrow_invocable<destroy_fn, Alloc&, T*>;

    public:
        using typename base::allocator_type;
        using typename base::const_pointer;
        using typename base::const_void_pointer;
        using typename base::difference_type;
        using typename base::is_always_equal;
        using typename base::pointer;
        using typename base::propagate_on_container_copy_assignment;
        using typename base::propagate_on_container_move_assignment;
        using typename base::propagate_on_container_swap;
        using typename base::size_type;
        using typename base::value_type;
        using typename base::void_pointer;

        using enum allocator_assign_operation;
        using enum allocator_swap_operation;

        struct allocation
        {
        private:
            pointer ptr_{};
            size_type size_{};

        public:
            [[nodiscard]] constexpr auto& ptr() const noexcept { return ptr_; }

            [[nodiscard]] constexpr auto size() const noexcept { return size_; }

            constexpr void deallocate(allocator_type& alloc) const noexcept
            {
                allocator_traits::deallocate(alloc, ptr_, size_);
            }

            constexpr void allocate(allocator_type& alloc, const size_type size) const noexcept
            {
                if(ptr_ != nullptr) allocator_traits::deallocate(alloc, ptr_, size);

                *this = allocator_traits::get_allocation(size);
            }

            [[nodiscard]] constexpr operator bool() const noexcept { return ptr_ != nullptr; }
        };

        template<typename U>
        using rebind_alloc = typename base::template rebind_alloc<U>;

        template<typename U>
        using rebind_traits = typename base::template rebind_traits<U>;

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

        static constexpr void
            deallocate(allocator_type& alloc, pointer ptr, const size_type count) noexcept
        {
            base::deallocate(alloc, ptr, count);
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

        static constexpr auto try_allocate(
            allocator_type& alloc,
            const size_type count,
            [[maybe_unused]] const const_void_pointer hint = nullptr
        ) noexcept
            requires requires // clang-format off
        {
            { alloc.try_allocate(count) } -> ::std::same_as<pointer>; // clang-format on
            requires noexcept(alloc.try_allocate(count));
        }
        {
            if constexpr( // clang-format off
                requires
                {
                    { alloc.try_allocate(count, hint) } ->
                        ::std::same_as<pointer>; // clang-format on
                    requires noexcept(alloc.try_allocate(count, hint));
                } //
            )
                if(hint != nullptr) return alloc.try_allocate(count, hint);

            return alloc.try_allocate(count);
        }

        static constexpr auto allocate_at_least(allocator_type& alloc, const size_type count)
        {
            return ::std::allocate_at_least(alloc, count);
        }

    private:
        template<bool IsCopy>
        using other_allocator_type =
            ::std::conditional_t<IsCopy, const allocator_type&, allocator_type>;

        template<typename Op, bool IsCopy>
        static constexpr auto assign_op_invocable = //
            ::std::invocable<
                Op,
                constant<before_assign>,
                allocator_type&,
                other_allocator_type<IsCopy>> &&
            ::std::invocable<
                Op,
                constant<after_assign>,
                allocator_type&,
                other_allocator_type<IsCopy>>;

        template<typename Op, bool IsCopy>
        static constexpr auto nothrow_assign_op_invocable = //
            nothrow_invocable<
                Op,
                constant<before_assign>,
                allocator_type&,
                other_allocator_type<IsCopy>> &&
            nothrow_invocable<
                Op,
                constant<after_assign>,
                allocator_type&,
                other_allocator_type<IsCopy>>;

    public:
        static constexpr struct assign_fn
        {
            template<typename Operation>
                requires assign_op_invocable<Operation, false> &&
                propagate_on_container_move_assignment::value
            constexpr void operator()(
                allocator_type& left,
                allocator_type&& right,
                Operation op //
            ) const noexcept(nothrow_assign_op_invocable<Operation, false>)
            {
                ::std::invoke(op, constant<before_assign>{}, left, ::std::move(right));
                left = ::std::move(right);
                ::std::invoke(op, constant<after_assign>{}, left, ::std::move(right));
            }

            template<::std::invocable<allocator_type&, allocator_type> Operation>
                requires(!propagate_on_container_move_assignment::value)
            constexpr void operator()(
                allocator_type& left,
                allocator_type&& right,
                Operation&& op //
            ) const noexcept(nothrow_invocable<Operation, allocator_type&, allocator_type>)
            {
                ::std::invoke(::std::forward<Operation>(op), left, ::std::move(right));
            }

            template<typename Operation>
                requires assign_op_invocable<Operation, true> &&
                propagate_on_container_copy_assignment::value
            constexpr void
                operator()(allocator_type& left, const allocator_type& right, Operation op) const
                noexcept(nothrow_assign_op_invocable<Operation, true>)
            {
                ::std::invoke(op, constant<before_assign>{}, left, right);
                left = right;
                ::std::invoke(op, constant<after_assign>{}, left, right);
            }

            template<::std::invocable<allocator_type&, const allocator_type&> Operation>
                requires(!propagate_on_container_copy_assignment::value)
            constexpr void
                operator()(allocator_type& left, const allocator_type& right, Operation&& op) const
                noexcept(nothrow_invocable<Operation, allocator_type&, const allocator_type&>)
            {
                ::std::invoke(::std::forward<Operation>(op), left, right);
            }
        } assign;

        template<typename... Args>
        static constexpr auto assign_req = //
            ::std::invocable<assign_fn, allocator_type&, Args...> ?
            nothrow_invocable<assign_fn, allocator_type&, Args...> ? //
                expr_req::no_exception :
                expr_req::well_formed :
            expr_req::ill_formed;

        static constexpr struct swap_fn
        {
            template<
                ::std::invocable<constant<before_swap>, allocator_type&, allocator_type&> Operation>
                requires ::std::invocable<
                             Operation,
                             constant<after_swap>,
                             allocator_type&,
                             allocator_type&> &&
                propagate_on_container_swap::value
            constexpr void operator()(
                allocator_type& left,
                allocator_type& right,
                Operation op
            ) const noexcept( //
                nothrow_invocable<
                    Operation,
                    constant<before_swap>,
                    allocator_type&,
                    allocator_type // clang-format off
                    >&& // clang-format on
                    nothrow_invocable<
                        Operation,
                        constant<after_swap>,
                        allocator_type&,
                        allocator_type& // clang-format off
                        > // clang-format on
            )
            {
                ::std::invoke(op, constant<before_swap>{}, left, right);
                ::std::ranges::swap(left, right);
                ::std::invoke(op, constant<after_swap>{}, left, right);
            }

            template<::std::invocable<allocator_type&, allocator_type&> Operation>
                requires(!propagate_on_container_swap::value)
            constexpr void
                operator()(allocator_type& left, allocator_type& right, Operation&& op) const
                noexcept(nothrow_invocable<Operation, allocator_type&, allocator_type&>)
            {
                ::std::invoke(::std::forward<Operation>(op), left, right);
            }
        } swap{};

        template<typename... Args>
        static constexpr auto swap_req = //
            ::std::invocable<swap_fn, allocator_type&, Args...> ?
            nothrow_invocable<swap_fn, allocator_type&, Args...> ? //
                expr_req::no_exception :
                expr_req::well_formed :
            expr_req::ill_formed;

        static constexpr decltype(auto) copy_construct(const allocator_type& alloc) noexcept
        {
            return select_on_container_copy_construction(alloc);
        }

        static constexpr allocation get_allocation(
            allocator_type& alloc,
            const size_type size,
            const const_void_pointer hint = nullptr
        )
        {
            return {size > 0 ? allocate(alloc, size, hint) : nullptr, size};
        }

        static constexpr allocation try_get_allocation(
            allocator_type& alloc,
            const size_type size,
            const const_void_pointer hint = nullptr
        ) noexcept
        {
            return {size > 0 ? try_allocate(alloc, size, hint) : nullptr, size};
        }

        template<typename T>
        static constexpr special_mem_req mem_req_for{
            constructible_from<T, T> ?
                nothrow_constructible_from<T, T> ? expr_req::no_exception : expr_req::well_formed :
                expr_req::ill_formed,
            constructible_from<T, const T&> ? //
                nothrow_constructible_from<T, const T&> ? //
                    expr_req::no_exception :
                    expr_req::well_formed :
                expr_req::ill_formed,
            move_assignable<T> ?
                nothrow_move_assignable<T> ? expr_req::no_exception : expr_req::well_formed :
                expr_req::ill_formed,
            copy_assignable<T> ?
                nothrow_copy_assignable<T> ? expr_req::no_exception : expr_req::well_formed :
                expr_req::ill_formed,
            destructible<T> ?
                nothrow_destructible<T> ? expr_req::no_exception : expr_req::well_formed :
                expr_req::ill_formed,
            ::std::swappable<T> ?
                nothrow_swappable<T> ? expr_req::no_exception : expr_req::well_formed :
                expr_req::ill_formed //
        };
    };

    template<typename>
    struct allocator_of;

    template<typename T>
        requires requires { typename T::allocator_type; }
    struct allocator_of<T> : ::std::type_identity<typename T::allocator_type>
    {
    };

    template<typename T>
    using allocator_of_t = ::meta::_t<allocator_of<T>>;
} // namespace stdsharp