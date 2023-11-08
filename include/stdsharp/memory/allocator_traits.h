#pragma once

#include <iterator>

#include "../type_traits/special_member.h"
#include "../utility/constructor.h"
#include "pointer_traits.h"

namespace stdsharp::details
{
    struct allocator_concept_traits
    {
    private:
        struct shadow_type
        {
            int v;
        };

    public:
        template<typename Traits>
        using rebind_traits = Traits::template rebind_traits<shadow_type>;
    };

    template<typename T>
    concept is_allocator_pointer =
        nullable_pointer<T> && nothrow_default_initializable<T> && nothrow_movable<T> &&
        nothrow_swappable<T> && nothrow_copyable<T> && nothrow_weakly_equality_comparable<T> &&
        nothrow_weakly_equality_comparable_with<T, std::nullptr_t>;

    template<typename T>
    struct make_obj_using_allocator_fn
    {
        template<typename Alloc, typename... Args>
            requires std::constructible_from<T, Args..., const Alloc&>
        constexpr T operator()(const Alloc& alloc, Args&&... args) const
            noexcept(nothrow_constructible_from<T, Args..., const Alloc&>)
        {
            return T{cpp_forward(args)..., alloc};
        }

        template<typename Alloc, typename... Args>
            requires std::constructible_from<T, std::allocator_arg_t, const Alloc&, Args...>
        constexpr T operator()(const Alloc& alloc, Args&&... args) const
            noexcept(nothrow_constructible_from<T, std::allocator_arg_t, const Alloc&, Args...>)
        {
            return T{std::allocator_arg, alloc, cpp_forward(args)...};
        }
    };
}

namespace stdsharp
{
    using max_alignment = std::alignment_of<std::max_align_t>;

    inline constexpr auto max_alignment_v = max_alignment::value;

    template<typename Alloc>
    concept allocator_req = requires //
    {
        typename Alloc::value_type;
        nothrow_copyable<Alloc>;

        requires requires(
            Alloc alloc,
            std::allocator_traits<Alloc> t_traits,
            decltype(t_traits)::pointer p,
            decltype(t_traits)::const_pointer const_p,
            decltype(t_traits)::void_pointer void_p,
            decltype(t_traits)::const_void_pointer const_void_p,
            decltype(t_traits)::value_type v,
            decltype(t_traits)::size_type size,
            details::allocator_concept_traits::rebind_traits<decltype(t_traits)> u_traits,
            decltype(t_traits)::is_always_equal always_equal,
            decltype(t_traits)::propagate_on_container_copy_assignment copy_assign,
            decltype(t_traits)::propagate_on_container_move_assignment move_assign,
            decltype(t_traits)::propagate_on_container_swap swap
        ) {
            requires !const_volatile<decltype(v)>;

            requires details::is_allocator_pointer<decltype(p)> &&
                details::is_allocator_pointer<decltype(const_p)> &&
                details::is_allocator_pointer<decltype(void_p)> &&
                details::is_allocator_pointer<decltype(const_void_p)>;

            requires std::random_access_iterator<decltype(p)> &&
                std::contiguous_iterator<decltype(p)>;

            requires nothrow_convertible_to<decltype(p), decltype(const_p)> &&
                std::random_access_iterator<decltype(const_p)> &&
                std::contiguous_iterator<decltype(const_p)>;

            requires nothrow_convertible_to<decltype(p), decltype(void_p)> &&
                nothrow_explicitly_convertible<decltype(void_p), decltype(p)> &&
                std::same_as<decltype(void_p), typename decltype(u_traits)::void_pointer>;

            requires nothrow_convertible_to<decltype(p), decltype(const_void_p)> &&
                nothrow_convertible_to<decltype(const_p), decltype(const_void_p)> &&
                nothrow_explicitly_convertible<decltype(const_void_p), decltype(const_p)> &&
                nothrow_convertible_to<decltype(void_p), decltype(const_void_p)> &&
                std::same_as< // clang-format off
                    decltype(const_void_p),
                    typename decltype(u_traits)::const_void_pointer
                >; // clang-format on

            requires std::unsigned_integral<decltype(size)>;

            requires std::signed_integral<typename decltype(t_traits)::difference_type>;

            requires std::
                same_as<typename decltype(u_traits)::template rebind_alloc<decltype(v)>, Alloc>;

            {
                *p
            } -> std::same_as<std::add_lvalue_reference_t<decltype(v)>>;
            {
                *const_p
            } -> std::same_as<add_const_lvalue_ref_t<decltype(v)>>;
            {
                *const_p
            } -> std::same_as<add_const_lvalue_ref_t<decltype(v)>>;

            requires requires(
                decltype(u_traits)::pointer other_p,
                decltype(u_traits)::const_pointer other_const_p,
                decltype(u_traits)::allocator_type u_alloc
            ) {
                nothrow_constructible_from<Alloc, const decltype(u_alloc)&>;
                nothrow_constructible_from<Alloc, decltype(u_alloc)>;

                {
                    other_p->v
                } -> std::same_as<decltype(((*other_p).v))>;
                {
                    other_const_p->v
                } -> std::same_as<decltype(((*other_const_p).v))>;

                t_traits.construct(alloc, other_p);
                t_traits.destroy(alloc, other_p);
            };

            requires noexcept(alloc == alloc)&& noexcept(alloc != alloc);

            pointer_traits<decltype(p)>::pointer_to(v);

            {
                t_traits.allocate(alloc, size)
            } -> std::same_as<decltype(p)>;
            {
                t_traits.allocate(alloc, size, const_void_p)
            } -> std::same_as<decltype(p)>;

            {
                alloc.deallocate(p, size)
            } noexcept;

            {
                t_traits.max_size(alloc)
            } noexcept -> std::same_as<decltype(size)>;

            requires std::derived_from<decltype(always_equal), std::true_type> ||
                std::derived_from<decltype(always_equal), std::false_type>;

            {
                t_traits.select_on_container_copy_construction(alloc)
            } -> std::same_as<Alloc>;

            requires std::derived_from<decltype(copy_assign), std::true_type> &&
                    nothrow_copy_assignable<Alloc> ||
                    std::derived_from<decltype(copy_assign), std::false_type>;

            requires std::derived_from<decltype(move_assign), std::true_type> &&
                    nothrow_move_assignable<Alloc> ||
                    std::derived_from<decltype(move_assign), std::false_type>;

            requires std::derived_from<decltype(swap), std::true_type> &&
                    nothrow_swappable<Alloc> || std::derived_from<decltype(swap), std::false_type>;
        };
    };

    template<typename T>
    using make_obj_uses_allocator_fn =
        sequenced_invocables<details::make_obj_using_allocator_fn<T>, constructor<T>>;

    template<typename T>
    inline constexpr make_obj_uses_allocator_fn<T> make_obj_uses_allocator{};

    template<auto = -1>
    struct allocator_propagation
    {
    };

    template<bool IsEqual>
    struct allocator_propagation<IsEqual>
    {
        static constexpr auto is_equal = IsEqual;

        bool assigned;
    };

    template<allocator_req Alloc>
    struct allocator_traits : private std::allocator_traits<Alloc>
    {
    private: // NOLINTBEGIN(*-owning-memory)
        struct default_constructor
        {
            template<typename T, typename... Args>
            constexpr decltype(auto) operator()( //
                const Alloc& /*unused*/,
                T* const ptr,
                Args&&... args
            ) const noexcept(stdsharp::nothrow_constructible_from<T, Args...>)
                requires std::constructible_from<T, Args...>
            {
                assert_not_null(ptr);
                return std::ranges::construct_at(ptr, cpp_forward(args)...);
            }

            template<typename T>
            constexpr decltype(auto) operator()(
                const Alloc& /*unused*/,
                void* const ptr,
                const std::in_place_type_t<T> /*unused*/,
                auto&&... args
            ) const noexcept(noexcept(::new(ptr) T{cpp_forward(args)...}))
                requires requires { ::new(ptr) T{cpp_forward(args)...}; }
            {
                assert_not_null(ptr);
                return ::new(ptr) T{cpp_forward(args)...};
            }
        };

        struct using_alloc_ctor
        {
            template<typename T, typename... Args>
            constexpr decltype(auto) operator()(
                const Alloc &
                    alloc,
                T* const ptr,
                Args&&... args //
            ) const noexcept(stdsharp::nothrow_constructible_from<T, Args..., const Alloc&>)
                requires std::constructible_from<T, Args..., const Alloc&>
            {
                assert_not_null(ptr);
                return std::ranges::construct_at(ptr, cpp_forward(args)..., alloc);
            }

            template<typename T, typename... Args, typename Tag = std::allocator_arg_t>
            constexpr decltype(auto) operator()(
                const Alloc &
                    alloc,
                T* const ptr,
                Args&&... args //
            ) const noexcept(stdsharp::nothrow_constructible_from<T, Tag, const Alloc&, Args...>)
                requires std::constructible_from<T, Tag, const Alloc&, Args...>
            {
                assert_not_null(ptr);
                return std::ranges::construct_at(
                    ptr,
                    std::allocator_arg,
                    alloc,
                    cpp_forward(args)...
                );
            }

            template<typename T>
            constexpr decltype(auto) operator()(
                const Alloc &
                    alloc,
                void* const ptr,
                const std::in_place_type_t<T> /*unused*/,
                auto&&... args
            ) const noexcept(noexcept(::new(ptr) T{cpp_forward(args)..., alloc}))
                requires requires {
                    ::new(ptr) T{cpp_forward(args)..., alloc};
                }
            {
                assert_not_null(ptr);
                return ::new(ptr) T{cpp_forward(args)..., alloc};
            }

            template<typename T>
            constexpr decltype(auto) operator()(
                const Alloc &
                    alloc,
                void* const ptr,
                const std::in_place_type_t<T> /*unused*/,
                auto&&... args
            ) const
                noexcept(noexcept(::new(ptr) T{std::allocator_arg, alloc, cpp_forward(args)...}))
                requires requires {
                    ::new(ptr) T{std::allocator_arg, alloc, cpp_forward(args)...};
                }
            {
                assert_not_null(ptr);
                return ::new(ptr) T{std::allocator_arg, alloc, cpp_forward(args)...};
            }
        };

        struct custom_constructor
        {
            constexpr decltype(auto) operator()(Alloc & a, auto* const ptr, auto&&... args) const
                noexcept(noexcept(a.construct(ptr, cpp_forward(args)...)))
                requires requires { a.construct(ptr, cpp_forward(args)...); }
            {
                assert_not_null(ptr);
                return a.construct(ptr, cpp_forward(args)...);
            }
        }; // NOLINTEND(*-owning-memory)

        using m_base = std::allocator_traits<Alloc>;

    public:
        using constructor =
            sequenced_invocables<custom_constructor, using_alloc_ctor, default_constructor>;

        static constexpr constructor construct{};

    private:
        struct custom_destructor
        {
            template<typename T>
            constexpr void operator()(Alloc& a, T* const ptr) const noexcept
                requires(noexcept(a.destroy(ptr)))
            {
                assert_not_null(ptr);
                a.destroy(ptr);
            }
        };

        struct default_destructor
        {
            template<typename T>
            constexpr void operator()(const Alloc& /*unused*/, T* const ptr) const noexcept
            {
                assert_not_null(ptr);
                std::ranges::destroy_at(ptr);
            }
        };

    public:
        using destructor = sequenced_invocables<custom_destructor, default_destructor>;

        static constexpr destructor destroy{};

        using typename m_base::allocator_type;
        using typename m_base::const_pointer;
        using typename m_base::const_void_pointer;
        using typename m_base::difference_type;
        using typename m_base::is_always_equal;
        using typename m_base::pointer;
        using typename m_base::propagate_on_container_copy_assignment;
        using typename m_base::propagate_on_container_move_assignment;
        using typename m_base::propagate_on_container_swap;
        using typename m_base::size_type;
        using typename m_base::value_type;
        using typename m_base::void_pointer;

#if __cpp_lib_allocate_at_least >= 202302L
        using m_base::allocate_at_least;
#endif

        static constexpr auto propagate_on_copy_v = propagate_on_container_copy_assignment::value;

        static constexpr auto propagate_on_move_v = propagate_on_container_move_assignment::value;

        static constexpr auto propagate_on_swap_v = propagate_on_container_swap::value;

        static constexpr auto always_equal_v = is_always_equal::value;

        template<typename U>
        using rebind_alloc = m_base::template rebind_alloc<U>;

        template<typename U>
        using rebind_traits = m_base::template rebind_traits<U>;

        using m_base::max_size;

        static constexpr allocator_type
            select_on_container_copy_construction(const allocator_type& alloc) noexcept
        {
            return alloc;
        }

        static constexpr allocator_type
            select_on_container_copy_construction(const allocator_type& alloc) //
            noexcept(noexcept(alloc.select_on_container_copy_construction()))
            requires requires {
                {
                    alloc.select_on_container_copy_construction()
                } -> std::convertible_to<allocator_type>;
            }
        {
            return alloc.select_on_container_copy_construction();
        }

        static constexpr pointer allocate(
            allocator_type& alloc,
            const size_type count,
            const const_void_pointer hint = nullptr
        )
        {
            return hint == nullptr ? m_base::allocate(alloc, count) :
                                     m_base::allocate(alloc, count, hint);
        }

        static constexpr void deallocate(allocator_type&, std::nullptr_t, size_type) = delete;

        static constexpr void
            deallocate(allocator_type& alloc, const pointer ptr, const size_type count) noexcept
        {
            assert_not_null(ptr);
            m_base::deallocate(alloc, ptr, count);
        }

        static constexpr pointer try_allocate(
            allocator_type& alloc,
            const size_type count,
            const const_void_pointer hint = nullptr
        ) noexcept
        {
            if(max_size(alloc) < count) return pointer{};

            try
            {
                return allocate(alloc, count, hint);
            }
            catch(...)
            {
                return pointer{};
            }
        }

        static constexpr auto try_allocate(
            allocator_type& alloc,
            const size_type count,
            [[maybe_unused]] const const_void_pointer hint = nullptr
        ) noexcept
            requires requires {
                {
                    alloc.try_allocate(count)
                } noexcept -> std::same_as<pointer>;
            }
        {
            if constexpr(requires {
                             {
                                 alloc.try_allocate(count, hint)
                             } noexcept -> std::same_as<pointer>;
                         })
                if(hint != nullptr) return alloc.try_allocate(count, hint);

            return alloc.try_allocate(count);
        }

        template<bool Propagate, bool IsEqual>
        using on_assign =
            std::conditional_t<Propagate, allocator_propagation<IsEqual>, allocator_propagation<>>;

        struct adaptor : allocator_type
        {
            adaptor() = default;

            [[nodiscard]] constexpr allocator_type& get_allocator() noexcept { return *this; }

            [[nodiscard]] constexpr const allocator_type& get_allocator() const noexcept
            {
                return *this;
            }

            constexpr adaptor(const std::in_place_t /*unused*/, auto&&... args) //
                noexcept(noexcept(allocator_type(cpp_forward(args)...)))
                requires requires { allocator_type(cpp_forward(args)...); }
                : allocator_type(cpp_forward(args)...)
            {
            }

            constexpr adaptor(const allocator_type& other) noexcept:
                allocator_type(select_on_container_copy_construction(other))
            {
            }

            constexpr adaptor(allocator_type&& other) noexcept: allocator_type(cpp_move(other)) {}

        private:
            template<std::copy_constructible Fn, typename OnAssign = on_assign<true, true>>
                requires std::invocable<Fn&, OnAssign, allocator_type&>
            constexpr void equal_assign(auto&& other, Fn& fn) noexcept(
                nothrow_copy_constructible<Fn> &&
                nothrow_invocable<Fn&, OnAssign, allocator_type&> //
            )
            {
                std::invoke(fn, OnAssign{false}, get_allocator());
                get_allocator() = cpp_forward(other);
                std::invoke(fn, OnAssign{true}, get_allocator());
            }

            template<std::copy_constructible Fn, typename OnAssign = on_assign<true, false>>
                requires std::invocable<Fn&, OnAssign, allocator_type&>
            constexpr void not_equal_assign(auto&& other, Fn& fn) noexcept(
                nothrow_copy_constructible<Fn> &&
                nothrow_invocable<Fn&, OnAssign, allocator_type&> //
            )
            {
                std::invoke(fn, OnAssign{false}, get_allocator());
                get_allocator() = cpp_forward(other);
                std::invoke(fn, OnAssign{true}, get_allocator());
            }

            template<std::copy_constructible Fn, typename OnAssign = on_assign<false, false>>
                requires std::invocable<Fn&, OnAssign, allocator_type&>
            constexpr void no_assign(Fn& fn) //
                noexcept(nothrow_copy_constructible<Fn> && nothrow_invocable<Fn&, OnAssign, allocator_type&>)
            {
                std::invoke(fn, OnAssign{}, get_allocator());
            }

        public:
            constexpr void assign(const allocator_type& other, auto fn) //
                noexcept(noexcept(equal_assign(other, fn)))
                requires requires {
                    requires propagate_on_copy_v;
                    requires always_equal_v;
                    equal_assign(other, fn);
                }
            {
                equal_assign(other, fn);
            }

            constexpr void assign(allocator_type&& other, auto fn) //
                noexcept(noexcept(equal_assign(cpp_move(other), fn)))
                requires requires {
                    requires propagate_on_move_v;
                    requires always_equal_v;
                    equal_assign(cpp_move(other), fn);
                }
            {
                equal_assign(cpp_move(other), fn);
            }

            constexpr void assign(const allocator_type& other, auto fn) //
                noexcept(noexcept(equal_assign(other, fn), not_equal_assign(other, fn)))
                requires requires {
                    requires propagate_on_copy_v;
                    equal_assign(other, fn);
                    not_equal_assign(other, fn);
                }
            {
                if(get_allocator() == other) equal_assign(other, fn);
                else not_equal_assign(other, fn);
            }

            constexpr void assign(allocator_type&& other, auto fn) //
                noexcept( //
                    noexcept(
                        equal_assign(cpp_move(other), fn),
                        not_equal_assign(cpp_move(other), fn)
                    )
                )
                requires requires {
                    requires propagate_on_move_v;
                    equal_assign(cpp_move(other), fn);
                    not_equal_assign(cpp_move(other), fn);
                }
            {
                if(get_allocator() == other) equal_assign(cpp_move(other), fn);
                else not_equal_assign(cpp_move(other), fn);
            }

            constexpr void assign(const allocator_type& /*unused*/, auto fn) //
                noexcept(noexcept(no_assign(fn)))
                requires requires { no_assign(fn); }
            {
                no_assign(fn);
            }

            constexpr void swap_with(allocator_type& other) noexcept
            {
                if constexpr(allocator_traits::propagate_on_swap_v)
                    std::ranges::swap(get_allocator(), other.get_allocator());
                else if constexpr(!always_equal_v)
                    Expects(get_allocator() == other.get_allocator());
            }
        };
    };

    template<allocator_req Alloc>
    using allocator_pointer = allocator_traits<Alloc>::pointer;

    template<allocator_req Alloc>
    using allocator_const_pointer = allocator_traits<Alloc>::const_pointer;

    template<allocator_req Alloc>
    using allocator_value_type = allocator_traits<Alloc>::value_type;

    template<allocator_req Alloc>
    using allocator_size_type = allocator_traits<Alloc>::size_type;

    template<allocator_req Alloc>
    using allocator_cvp = allocator_traits<Alloc>::const_void_pointer;

    template<allocator_req Alloc>
    using allocator_void_pointer = allocator_traits<Alloc>::void_pointer;

    template<allocator_req Alloc>
    using allocator_difference_type = allocator_traits<Alloc>::difference_type;

    template<typename>
    struct allocator_of;

    template<typename T>
        requires requires { typename T::allocator_type; }
    struct allocator_of<T> : std::type_identity<typename T::allocator_type>
    {
    };

    template<typename T>
    using allocator_of_t = ::meta::_t<allocator_of<T>>;
}