#pragma once

#include "pointer_traits.h"
#include "allocator_traits.h"
#include "../utility/implementation_reference.h"
#include "../cassert/cassert.h"

namespace stdsharp
{
    namespace details
    {
        template<expr_req Req, typename... Args>
        using special_mem_reference = implementation_reference<Req, void, Args...>;

        template<special_mem_req Req, special_mem_req Other>
        static constexpr bool req_compatible = Req <= Other;

        template<typename T, typename Allocator, special_mem_req Req>
        concept req_compatible_for =
            req_compatible<Req, allocator_traits<Allocator>::template mem_req_for<T>>;

        template<special_mem_req Req, allocator_req Alloc>
        struct special_mem_traits_base
        {
            using alloc_traits = allocator_traits<Alloc>;
            using pointer = typename alloc_traits::pointer;
            using const_pointer = typename alloc_traits::const_pointer;
            using ptr_cref = const pointer&;
            using cptr_cref = const const_pointer&;

            static constexpr auto move_ctor_req = Req.move_construct;
            static constexpr auto cp_ctor_req = Req.copy_construct;
            static constexpr auto mov_assign_req = Req.move_assign;
            static constexpr auto cp_assign_req = Req.copy_assign;
            static constexpr auto dtor_req = Req.destruct;
            static constexpr auto swp_req = Req.swap;

            using base = stdsharp::indexed_values<
                details::special_mem_reference<move_ctor_req, Alloc&, ptr_cref, ptr_cref>,
                details::special_mem_reference<cp_ctor_req, Alloc&, ptr_cref, cptr_cref>,
                details::special_mem_reference<mov_assign_req, ptr_cref, ptr_cref>,
                details::special_mem_reference<cp_assign_req, ptr_cref, cptr_cref>,
                details::special_mem_reference<dtor_req, Alloc&, ptr_cref>,
                details::special_mem_reference<swp_req, ptr_cref, ptr_cref> // clang-format off
            >; // clang-format on

            class mem_traits : base
            {
                constexpr const base& to_base() const noexcept { return *this; }

            protected:
                constexpr mem_traits(
                    const base& b,
                    const ::std::string_view current_type,
                    const ::std::size_t type_size
                ) noexcept:
                    base(b), current_type_(current_type), type_size_(type_size)
                {
                }

            public:
                static constexpr auto req = Req;

                mem_traits() = default;

                template<req_compatible_for<Alloc, Req> T>
                constexpr mem_traits(const ::std::type_identity<T>) noexcept:
                    mem_traits(
                        base(
                            [](Alloc & alloc, ptr_cref left, ptr_cref right) //
                            noexcept(move_ctor_req == expr_req::no_exception) //
                            requires(move_ctor_req >= expr_req::well_formed) //
                            {
                                alloc_traits::construct(
                                    alloc,
                                    pointer_cast<T>(left),
                                    *pointer_cast<T>(right)
                                );
                            },
                            [](Alloc & alloc, ptr_cref left, cptr_cref right) //
                            noexcept(cp_ctor_req == expr_req::no_exception) //
                            requires(cp_ctor_req >= expr_req::well_formed) //
                            {
                                alloc_traits::construct(
                                    alloc,
                                    pointer_cast<T>(left),
                                    *pointer_cast<T>(right)
                                );
                            },
                            [](ptr_cref left, ptr_cref right) //
                            noexcept(mov_assign_req == expr_req::no_exception) //
                            requires(mov_assign_req >= expr_req::well_formed) //
                            {
                                *pointer_cast<T>(left) = ::std::move(*pointer_cast<T>(right)); //
                            },
                            [](ptr_cref left, cptr_cref right) //
                            noexcept(cp_assign_req == expr_req::no_exception) //
                            requires(cp_assign_req >= expr_req::well_formed) //
                            {
                                *pointer_cast<T>(left) = *pointer_cast<T>(right); //
                            },
                            [](Alloc & alloc, ptr_cref left) //
                            noexcept(dtor_req == expr_req::no_exception) //
                            requires(dtor_req >= expr_req::well_formed) //
                            {
                                alloc_traits::destroy(alloc, pointer_cast<T>(left)); //
                            },
                            [](ptr_cref left, ptr_cref right) //
                            noexcept(swp_req == expr_req::no_exception) //
                            requires(swp_req >= expr_req::well_formed) //
                            {
                                ::std::ranges::swap(
                                    *pointer_cast<T>(left),
                                    *pointer_cast<T>(right)
                                ); //
                            }
                        ),
                        type_id<T>,
                        sizeof(T)
                    )
                {
                }

                constexpr void construct(Alloc& alloc, ptr_cref left, ptr_cref right) const
                    noexcept(move_ctor_req == expr_req::no_exception)
                    requires(move_ctor_req >= expr_req::well_formed)
                {
                    stdsharp::get<0>(to_base())(alloc, left, right);
                }

                constexpr void construct(Alloc& alloc, ptr_cref left, cptr_cref right) const
                    noexcept(cp_ctor_req == expr_req::no_exception)
                    requires(cp_ctor_req >= expr_req::well_formed)
                {
                    stdsharp::get<1>(to_base())(alloc, left, right);
                }

                constexpr void assign(ptr_cref left, ptr_cref right) const
                    noexcept(mov_assign_req == expr_req::no_exception)
                    requires(mov_assign_req >= expr_req::well_formed)
                {
                    stdsharp::get<2>(to_base())(left, right);
                }

                constexpr void assign(ptr_cref left, cptr_cref right) const
                    noexcept(cp_assign_req == expr_req::no_exception)
                    requires(cp_assign_req >= expr_req::well_formed)
                {
                    stdsharp::get<3>(to_base())(left, right);
                }

                constexpr void destruct(Alloc& alloc, ptr_cref ptr) const
                    noexcept(dtor_req == expr_req::no_exception)
                {
                    stdsharp::get<4>(to_base())(alloc, ptr);
                }

                // NOLINTBEGIN(*-magic-numbers)
                constexpr void do_swap(ptr_cref left, ptr_cref right) const
                    noexcept(swp_req == expr_req::no_exception)
                    requires(swp_req >= expr_req::well_formed)
                {
                    stdsharp::get<5>(to_base())(left, right);
                } // NOLINTEND(*-magic-numbers)

                [[nodiscard]] constexpr auto type() const noexcept { return current_type_; }

                [[nodiscard]] constexpr auto size() const noexcept { return type_size_; }

                [[nodiscard]] constexpr auto has_value() const noexcept { return size() != 0; }

            private:
                ::std::string_view current_type_ = type_id<void>;
                ::std::size_t type_size_{};
            };
        };

        template<special_mem_req Req, allocator_req Alloc>
        struct special_mem_traits : special_mem_traits_base<Req, Alloc>::mem_traits
        {
            using special_mem_traits_base<Req, Alloc>::mem_traits::mem_traits;

            template<special_mem_req OtherReq>
            constexpr special_mem_traits(const special_mem_traits<OtherReq, Alloc>& other) noexcept:
                special_mem_traits_base<Req, Alloc>::mem_traits(
                    other,
                    other.current_type_,
                    other.type_size_
                )
            {
            }
        };
    }

    template<special_mem_req Req, allocator_req Alloc>
        requires(Req.destruct >= expr_req::well_formed)
    class basic_object_allocation
    {
        using alloc_traits = allocator_traits<Alloc>;
        using pointer = typename alloc_traits::pointer;
        using const_pointer = typename alloc_traits::const_pointer;
        using size_type = typename alloc_traits::size_type;
        using alloc_construct_fn = typename alloc_traits::construct_fn;

        using ptr_cref = const pointer&;
        using cptr_cref = const const_pointer&;

        using mem_traits = typename details::special_mem_traits<Req, Alloc>;

        template<typename T>
        static constexpr mem_traits mem_traits_for{::std::type_identity<T>{}};

        Alloc allocator_{};
        typename alloc_traits::allocated allocated_{};
        mem_traits traits_{};

        [[nodiscard]] constexpr auto& get_ptr() const noexcept { return allocated_.ptr; }

        constexpr void destroy() noexcept(Req.destruct == expr_req::no_exception)
        {
            if(has_value())
            {
                traits_.destruct(allocator_, get_ptr());
                traits_ = {};
            }
        }

        constexpr void deallocate() noexcept
        {
            if(allocated_.ptr == nullptr) return;

            alloc_traits::deallocate(allocator_, get_ptr(), allocated_.size);
            allocated_ = {};
        }

        template<special_mem_req OtherReq>
        constexpr void
            assign_traits(const details::special_mem_traits<OtherReq, Alloc>& other_traits)
        {
            if(traits_.type() == other_traits.type()) return;

            if(!other_traits.has_value()) reset();

            const auto size = other_traits.size();

            destroy();

            if(allocated_.size < size)
            {
                deallocate();
                allocated_ = alloc_traits::get_allocated(allocator_, size);
            }

            traits_ = other_traits;
        }

        struct private_tag
        {
        };

        template<special_mem_req OtherReq>
        constexpr basic_object_allocation(
            const private_tag,
            const basic_object_allocation<OtherReq, Alloc>& other
        )
            requires(Req.copy_construct == expr_req::well_formed) &&
                        ::std::constructible_from<mem_traits, decltype(other.traits_)>
            :
            allocator_(alloc_traits::copy_construct(other.allocator_)),
            allocated_( //
                {
                    other.has_value() ? alloc_traits::allocate(allocator_, other.size) : nullptr,
                    other.size //
                }
            ),
            traits_(other.traits_)
        {
            if(other.has_value()) traits_.construct(allocator_, get_ptr(), other.get_ptr());
        }

        template<special_mem_req OtherReq>
        struct swap_op
        {
            basic_object_allocation& this_;
            basic_object_allocation<OtherReq, Alloc>& other;

            using enum allocator_swap_operation;

            constexpr void swap_allocated(basic_object_allocation& other) noexcept
            {
                ::std::swap(this_.allocated_, other.allocated_);
                ::std::swap(this_.traits_, other.traits_);
            };

            constexpr void operator()(const Alloc&, const Alloc&, const constant<before_swap>) //
                noexcept
            {
            }

            constexpr void
                operator()(const Alloc&, const Alloc&, const constant<after_swap>) noexcept
            {
                swap_allocated(other); //
            }

            constexpr void operator()(const Alloc& left, const Alloc& right) //
                noexcept(alloc_traits::is_always_equal::value || !is_debug)
            {
                if(left == right) swap_allocated();
                if constexpr(is_debug)
                    throw ::std::invalid_argument{"Swap with different allocators"};
            }
        };

        template<typename ValueType, typename... Args>
            requires move_assignable<ValueType>
        constexpr bool try_same_type_emplace(Args&&... args) //
            noexcept(nothrow_constructible_from<ValueType, Args...>&&
                         nothrow_move_assignable<ValueType>)
        {
            get<ValueType>() = ValueType{::std::forward<Args>(args)...};
            return true;
        }

        template<typename>
        constexpr bool try_same_type_emplace(const auto&...) noexcept
        {
            return false;
        }

        template<typename T>
        [[nodiscard]] constexpr auto ptr_cast() const noexcept
        {
            return pointer_cast<T>(get_ptr());
        }

    public:
        static constexpr auto req = Req;

        basic_object_allocation() = default;

        constexpr basic_object_allocation(const Alloc& alloc) noexcept: allocator_(alloc) {}

        template<
            typename... Args,
            typename T,
            typename ValueType = ::std::decay_t<T>,
            typename IdentityT = ::std::type_identity<ValueType> // clang-format off
        > // clang-format on
            requires ::std::constructible_from<mem_traits, IdentityT> &&
                         ::std::invocable<alloc_construct_fn, Alloc&, ValueType*, Args...>
        constexpr basic_object_allocation(
            const ::std::in_place_type_t<T>,
            Args&&... args,
            const Alloc& alloc = Alloc{}
        ):
            basic_object_allocation(alloc),
            allocated_(alloc_traits::get_allocated(allocator_, sizeof(ValueType))),
            traits_(IdentityT{})
        {
            alloc_traits::construct(allocator_, pt_cast<T>(), ::std::forward<Args>(args)...);
        }

        template<special_mem_req OtherReq>
        constexpr basic_object_allocation(const basic_object_allocation<OtherReq, Alloc>& other)
            requires ::std::
                is_constructible_v<basic_object_allocation, private_tag, decltype(other)>
            : basic_object_allocation(private_tag{}, other)
        {
        }

        template<special_mem_req OtherReq>
        constexpr basic_object_allocation(basic_object_allocation<OtherReq, Alloc>&& other) noexcept
            requires ::std::is_constructible_v<mem_traits, decltype(other.traits_)>
            :
            allocator_(::std::move(other.allocator_)),
            allocated_(other.allocated_),
            traits_(other.traits_)
        {
        }

        constexpr basic_object_allocation(const basic_object_allocation& other)
            requires ::std::
                is_constructible_v<basic_object_allocation, private_tag, decltype(other)>
            : basic_object_allocation(private_tag{}, other)
        {
        }

        basic_object_allocation(basic_object_allocation&&) noexcept = default;

        constexpr ~basic_object_allocation() noexcept(noexcept(destroy())) { reset(); }

        [[nodiscard]] constexpr auto get_allocator() const noexcept { return allocator_; }

        [[nodiscard]] constexpr auto type() const noexcept { return traits_.type(); }

        [[nodiscard]] constexpr auto size() const noexcept { return traits_.size(); }

        [[nodiscard]] constexpr auto reserved() const noexcept { return allocated_.size; }

        template<typename T>
        [[nodiscard]] constexpr auto& get() noexcept
        {
            return *ptr_cast<T>();
        }

        template<typename T>
        [[nodiscard]] constexpr const auto& get() const noexcept
        {
            return *ptr_cast<T>();
        }

        constexpr void reset() noexcept(noexcept(destroy()))
        {
            destroy();
            deallocate();
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return traits_.has_value(); }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }

        template<typename T, typename... Args>
            requires ::std::invocable<alloc_construct_fn, Alloc&, ::std::decay_t<T>*, Args...> &&
            details::req_compatible_for<::std::decay_t<T>, Alloc, Req>
        constexpr decltype(auto) emplace(Args&&... args)
        {
            if(type_id<T> == type() &&
               try_same_type_emplace<::std::decay_t<T>>(::std::forward<Args>(args)...))
                return get<T>();

            assign_traits(mem_traits_for<T>);

            alloc_traits::construct(
                allocator_,
                ptr_cast<::std::decay_t<T>>(),
                ::std::forward<Args>(args)...
            );

            return get<T>();
        }

    private:
        template<typename Ptr, special_mem_req OtherReq>
        constexpr void assign_ptr(
            const Ptr& other_ptr,
            const details::special_mem_traits<OtherReq, Alloc>& other_traits
        )
            requires requires //
        {
            requires details::req_compatible<Req, OtherReq>;
            traits_.construct(allocator_, get_ptr(), other_ptr);
        }
        {
            if(!other_traits.has_value())
            {
                if(has_value()) reset();
                return;
            }

            if(type() == other_traits.type())
            {
                if constexpr(requires { traits_.assign(get_ptr(), other_ptr); })
                {
                    traits_.assign(get_ptr(), other_ptr);
                    return;
                }
            }
            else assign_traits(other_traits);

            traits_.construct(allocator_, get_ptr(), other_ptr);
        }

        struct mem_assign_fn
        {
            template<special_mem_req OtherReq>
            constexpr void operator()(
                basic_object_allocation& left,
                const basic_object_allocation<OtherReq, Alloc>& right
            ) const
                requires requires { left.assign_ptr(left.get_ptr(), right.traits_); }
            {
                const auto copy_assign = [ // clang-format off
                    &left,
                    ptr = static_cast<const_pointer>(right.get_ptr()),
                    other_traits = right.traits_ // clang-format on
                ](const Alloc&, const Alloc&, const constant<alloc_traits::after_assign> = {})
                {
                    left.assign_ptr(ptr, other_traits); //
                };

                alloc_traits::assign(
                    left.allocator_,
                    right.allocator_,
                    make_invocables(
                        [&left](const Alloc& alloc_left, const Alloc& alloc_right, const constant<alloc_traits::before_assign>)
                        {
                            if(alloc_left != alloc_right) left.reset();
                        },
                        copy_assign,
                        copy_assign
                    )
                );
            }

            template<special_mem_req OtherReq>
            struct move_assign_op
            {
                basic_object_allocation& this_;
                basic_object_allocation<OtherReq, Alloc>&& other;

                constexpr void copy_allocated(basic_object_allocation&& other) noexcept
                {
                    this_.allocated_ = other.allocated_;
                    this_.traits_ = other.traits_;
                };

                constexpr void
                    operator()(const Alloc&, const Alloc&, const constant<alloc_traits::before_assign>) //
                    noexcept(noexcept(this_.reset()))
                {
                    this_.reset(); //
                }

                constexpr void
                    operator()(const Alloc&, const Alloc&, const constant<alloc_traits::after_assign>) noexcept
                {
                    copy_allocated(other); //
                }

                constexpr void operator()(const Alloc& left, const Alloc& right)
                    requires requires { this_.assign_ptr(other.get_ptr(), other.traits_); }
                {
                    if(left == right)
                    {
                        this_.reset();
                        copy_allocated();
                    }
                    else this_.assign_ptr(other.get_ptr(), other.traits_);
                }
            };

            template<special_mem_req OtherReq>
            constexpr void operator()(
                basic_object_allocation& left,
                basic_object_allocation<OtherReq, Alloc>&& right
            ) const noexcept( //
                noexcept( //
                    alloc_traits::assign(
                        left.allocator_,
                        ::std::move(right.allocator_),
                        move_assign_op{*this, ::std::move(right)}
                    )
                )
            )
                requires requires //
            {
                alloc_traits::assign(
                    left.allocator_,
                    ::std::move(right.allocator_),
                    move_assign_op{*this, ::std::move(right)}
                ); //
            }
            {
                alloc_traits::assign(
                    left.allocator_,
                    ::std::move(right.allocator_),
                    move_assign_op{*this, ::std::move(right)}
                );
            }
        };

        struct value_assign_fn
        {
            template<typename T>
            constexpr void operator()(basic_object_allocation& left, T&& t) //
                const noexcept(noexcept(left.emplace<T>(::std::declval<T>())))
                requires requires { left.emplace<T>(::std::declval<T>()); }
            {
                left.emplace<T>(::std::forward<T>(t));
                return *this;
            }
        };

        using assign_fn = sequenced_invocables<mem_assign_fn, value_assign_fn>;

        static constexpr assign_fn assign;

    public:
        template<typename T>
            requires ::std::invocable<assign_fn, basic_object_allocation&, T>
        constexpr basic_object_allocation& operator=(T&& t) //
            noexcept(nothrow_invocable<assign_fn, basic_object_allocation&, T>)
        {
            assign(*this, ::std::forward<T>(t));
            return *this;
        }

        constexpr basic_object_allocation& operator=(const basic_object_allocation& other) //
            noexcept(nothrow_invocable<
                     assign_fn,
                     basic_object_allocation&,
                     const basic_object_allocation&>)
            requires ::std::
                invocable<assign_fn, basic_object_allocation&, const basic_object_allocation&>
        {
            if(this == &other) return *this;
            assign(*this, other);
            return *this;
        }

        constexpr basic_object_allocation& operator=(basic_object_allocation&& other) //
            noexcept(nothrow_invocable<
                     assign_fn,
                     basic_object_allocation&,
                     basic_object_allocation>)
            requires ::std::invocable<assign_fn, basic_object_allocation&, basic_object_allocation>
        {
            if(this == &other) return *this;
            assign(*this, ::std::move(other));
            return *this;
        }

        template<typename ValueType, typename U, typename... Args>
        constexpr decltype(auto) emplace(const std::initializer_list<U> il, Args&&... args)
            requires requires //
        {
            emplace<ValueType, const std::initializer_list<U>, Args...>(
                il,
                ::std::declval<Args>()...
            );
        }
        {
            return emplace<ValueType, const std::initializer_list<U>, Args...>(
                il,
                ::std::forward<Args>(args)...
            );
        }

        template<typename T>
        constexpr decltype(auto) emplace(T&& t)
            requires requires { emplace<::std::decay_t<T>, T>(::std::declval<T>()); }
        {
            return emplace<::std::decay_t<T>, T>(::std::forward<T>(t));
        }

        template<special_mem_req OtherReq>
        constexpr void swap(basic_object_allocation<OtherReq, Alloc>& other) //
            noexcept(noexcept(alloc_traits::swap(allocator_, allocator_, swap_op{*this, other})))
            requires requires //
        {
            alloc_traits::swap(this->allocator_, other.allocator_, swap_op{*this, other});
        }
        {
            alloc_traits::swap(allocator_, other.allocator_, swap_op{*this, other});
        }
    };

    template<typename T, allocator_req Alloc>
    using object_allocation_like =
        basic_object_allocation<allocator_traits<Alloc>::template mem_req_for<T>, Alloc>;

    template<allocator_req Alloc>
    using trivial_object_allocation = basic_object_allocation<special_mem_req::trivial, Alloc>;

    template<allocator_req Alloc>
    using normal_movable_object_allocation =
        basic_object_allocation<special_mem_req::normal_movable, Alloc>;

    template<allocator_req Alloc>
    using normal_object_allocation = basic_object_allocation<
        special_mem_req{
            .move_construct = expr_req::well_formed,
            .copy_construct = expr_req::well_formed,
            .copy_assign = expr_req::well_formed //
        },
        Alloc>;
}