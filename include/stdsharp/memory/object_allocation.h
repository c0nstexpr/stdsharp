#pragma once

#include "pointer_traits.h"
#include "allocator_traits.h"
#include "../type_traits/object.h"
#include "../type_traits/special_member.h"
#include <functional>

namespace stdsharp
{
    template<expr_req ExprReq, typename Ret, typename... Arg>
    struct implementation_reference :
        ::std::reference_wrapper<Ret(Arg&&...) noexcept(ExprReq == expr_req::no_exception)>
    {
        static constexpr auto requirement = ExprReq;
    };

    template<typename Ret, typename... Arg>
    struct implementation_reference<expr_req::ill_formed, Ret, Arg...> : empty_t
    {
        static constexpr auto requirement = expr_req::ill_formed;
    };

    template<special_mem_req, typename>
    class basic_object_allocation;

    namespace details
    {
        template<expr_req Req, typename... Args>
        using special_mem_reference = implementation_reference<Req, void, Args...>;

        template<expr_req Req, typename Alloc>
        struct mov_ctor :
            special_mem_reference<
                Req,
                Alloc&,
                const typename allocator_traits<Alloc>::pointer&,
                const typename allocator_traits<Alloc>::pointer&>
        {
            using allocator_traits = allocator_traits<Alloc>;
            using pointer = typename allocator_traits::pointer;

            template<typename T>
            constexpr mov_ctor(const ::std::type_identity<T>) noexcept:
                special_mem_reference<Req, Alloc&, const pointer&, const pointer&>(
                    [](Alloc & alloc, const pointer& left, const pointer& right) //
                    noexcept(Req == expr_req::no_exception) //
                    requires(Req >= expr_req::well_formed) //
                    {
                        allocator_traits::construct(
                            alloc,
                            dereference_to<T>(left),
                            ::std::move(dereference_to<T>(right))
                        );
                    } //
                )
            {
            }
        };

        template<expr_req Req, typename Alloc>
        struct cp_ctor :
            special_mem_reference<
                Req,
                Alloc&,
                const typename allocator_traits<Alloc>::pointer&,
                const typename allocator_traits<Alloc>::const_pointer&>
        {
            using allocator_traits = allocator_traits<Alloc>;
            using pointer = typename allocator_traits::pointer;
            using const_pointer = typename allocator_traits::const_pointer;

            template<typename T>
            constexpr cp_ctor(const ::std::type_identity<T>) noexcept:
                special_mem_reference<Req, Alloc&, const pointer&, const const_pointer&>(
                    [](Alloc & alloc, const pointer& left, const const_pointer& right) //
                    noexcept(Req == expr_req::no_exception) //
                    requires(Req >= expr_req::well_formed) //
                    {
                        allocator_traits::construct(
                            alloc,
                            dereference_to<T>(left),
                            dereference_to<T>(right)
                        );
                    } //
                )
            {
            }
        };

        template<expr_req Req, typename Alloc>
        struct dtor :
            special_mem_reference<Req, Alloc&, const typename allocator_traits<Alloc>::pointer&>
        {
            using allocator_traits = allocator_traits<Alloc>;
            using pointer = typename allocator_traits::pointer;

            template<typename T>
            constexpr dtor(const ::std::type_identity<T>) noexcept:
                special_mem_reference<Req, Alloc&, const pointer&>(
                    [](Alloc & alloc, const pointer& ptr) //
                    noexcept(Req == expr_req::no_exception) //
                    requires(Req >= expr_req::well_formed) //
                    {
                        allocator_traits::destroy(alloc, dereference_to<T>(ptr)); //
                    }
                )
            {
            }
        };

        template<expr_req Req, typename Ptr>
        struct mov_assign : special_mem_reference<Req, const Ptr&, const Ptr&>
        {
            template<typename T>
            constexpr mov_assign(const ::std::type_identity<T>) noexcept:
                special_mem_reference<Req, const Ptr&, const Ptr&>(
                    [](const Ptr& left, const Ptr& right) //
                    noexcept(Req == expr_req::no_exception) //
                    requires(Req >= expr_req::well_formed) //
                    {
                        dereference_to<T>(left) = ::std::move(dereference_to<T>(right)); //
                    }
                )
            {
            }
        };

        template<expr_req Req, typename Ptr, typename ConstPtr>
        struct cp_assign : special_mem_reference<Req, const Ptr&, const ConstPtr&>
        {
            template<typename T>
            constexpr cp_assign(const ::std::type_identity<T>) noexcept:
                special_mem_reference<Req, const Ptr&, const ConstPtr&>(
                    [](const Ptr& left, const ConstPtr& right) //
                    noexcept(Req == expr_req::no_exception) //
                    requires(Req >= expr_req::well_formed) //
                    {
                        dereference_to<T>(left) = dereference_to<T>(right); //
                    }
                )
            {
            }
        };

        template<expr_req Req, typename Ptr>
        struct swap_impl : special_mem_reference<Req, const Ptr&, const Ptr&>
        {
            template<typename T>
            constexpr swap_impl(const ::std::type_identity<T>) noexcept:
                special_mem_reference<Req, const Ptr&, const Ptr&>(
                    [](const Ptr& left, const Ptr& right) //
                    noexcept(Req == expr_req::no_exception) //
                    requires(Req >= expr_req::well_formed) //
                    {
                        ::std::ranges::swap(dereference_to<T>(left), dereference_to<T>(right)); //
                    }
                )
            {
            }
        };

        template<special_mem_req Req, typename Alloc>
        struct special_member_traits
        {
            using allocator_traits = allocator_traits<Alloc>;
            using pointer = typename allocator_traits::pointer;
            using pointer_traits = pointer_traits<pointer>;
            using const_pointer = typename allocator_traits::const_pointer;

            using mov_ctor = mov_ctor<Req.move_construct, Alloc>;
            using cp_ctor = cp_ctor<Req.move_construct, Alloc>;
            using mov_assign = mov_assign<Req.move_assign, pointer>;
            using cp_assign = cp_assign<Req.copy_assign, pointer, const_pointer>;
            using dtor = dtor<Req.destruct, Alloc>;
            using swap_impl = swap_impl<Req.swap, pointer>;

            class impl : mov_ctor, cp_ctor, mov_assign, cp_assign, dtor, swap_impl
            {
            public:
                template<typename T>
                    requires(Req <= special_mem_req::for_type<T>())
                static const impl for_type;

                impl() = default;

                template<typename T>
                    requires(Req <= special_mem_req::for_type<T>())
                constexpr impl(const ::std::type_identity<T> v) noexcept:
                    current_type_(type_id<T>),
                    type_size_(sizeof(T)),
                    mov_ctor(v),
                    cp_ctor(v),
                    mov_assign(v),
                    cp_assign(v),
                    dtor(v),
                    swap_impl(v)
                {
                }

                template<special_mem_req OtherReq>
                    requires(Req <= OtherReq)
                constexpr impl( //
                    const typename special_member_traits<OtherReq, Alloc>::impl& other
                ) noexcept:
                    current_type_(other.current_type_),
                    type_size_(other.type_size_),
                    mov_ctor(other.mov_ctor),
                    cp_ctor(other.cp_ctor),
                    mov_assign(other.mov_assign),
                    cp_assign(other.cp_assign),
                    dtor(other.dtor)
                {
                }

                constexpr void
                    construct(Alloc& alloc, const pointer& left, const pointer& right) const
                    noexcept(nothrow_invocable<mov_ctor, Alloc, pointer, pointer>)
                    requires ::std::invocable<mov_ctor, Alloc, pointer, pointer>
                {
                    static_cast<const mov_ctor&>(*this)(alloc, left, right);
                }

                constexpr void
                    construct(Alloc& alloc, const pointer& left, const const_pointer& right) const
                    noexcept(nothrow_invocable<cp_ctor, Alloc, pointer, const_pointer>)
                    requires ::std::invocable<cp_ctor, Alloc, pointer, const_pointer>
                {
                    static_cast<const cp_ctor&>(*this)(alloc, left, right);
                }

                constexpr void assign(const pointer& left, const pointer& right) const
                    noexcept(nothrow_invocable<mov_assign, Alloc, pointer, const_pointer>)
                    requires ::std::invocable<mov_assign, Alloc, pointer, const_pointer>
                {
                    static_cast<const mov_assign&>(*this)(left, right);
                }

                constexpr void assign(const pointer& left, const const_pointer& right) const
                    noexcept(nothrow_invocable<cp_assign, Alloc, pointer, const_pointer>)
                    requires ::std::invocable<cp_assign, Alloc, pointer, const_pointer>
                {
                    static_cast<const cp_assign&>(*this)(left, right);
                }

                constexpr void destruct(Alloc& alloc, const pointer& ptr) const
                    noexcept(nothrow_invocable<dtor, Alloc, pointer>)
                    requires ::std::invocable<dtor, Alloc, pointer>
                {
                    static_cast<const dtor&>(*this)(alloc, ptr);
                }

                constexpr void do_swap(const pointer& left, const pointer& right) const
                    noexcept(nothrow_invocable<swap_impl, Alloc, pointer, pointer>)
                    requires ::std::invocable<swap_impl, Alloc, pointer, pointer>
                {
                    static_cast<const swap_impl&>(*this)(left, right);
                }

                [[nodiscard]] constexpr auto type() const noexcept { return current_type_; }

                [[nodiscard]] constexpr auto size() const noexcept { return type_size_; }

                [[nodiscard]] constexpr auto has_value() const noexcept
                {
                    return !current_type_.empty();
                }

            private:
                ::std::string_view current_type_ = type_id<void>;
                ::std::size_t type_size_{};
            };
        };

        template<special_mem_req Req, typename Alloc>
        using special_member_traits_t = typename special_member_traits<Req, Alloc>::impl;

        template<special_mem_req Req, typename Alloc>
        template<typename T>
            requires(Req <= special_mem_req::for_type<T>())
        inline constexpr special_member_traits_t<Req, Alloc> special_member_traits<Req, Alloc>::
            impl::for_type{::std::type_identity<T>{}};
    }

    template<special_mem_req Req, typename Alloc>
        requires(Req.destruct == expr_req::well_formed)
    class basic_object_allocation<Req, Alloc>
    {
        using alloc_traits = allocator_traits<Alloc>;
        using pointer = typename alloc_traits::pointer;
        using const_pointer = typename alloc_traits::const_pointer;
        using size_type = typename alloc_traits::size_type;
        using mem_traits = details::special_member_traits_t<Req, Alloc>;

        template<special_mem_req OtherReq>
        using other_mem_traits = details::special_member_traits_t<OtherReq, Alloc>;

        [[nodiscard]] constexpr auto& get_ptr() const noexcept { return allocated_.ptr; }

        constexpr void destroy() const noexcept(Req.destruct == expr_req::no_exception)
        {
            if(has_value())
            {
                traits_.destroy(get_ptr());
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
        constexpr void assign_traits(const other_mem_traits<OtherReq>& other_traits)
            requires ::std::assignable_from<mem_traits&, decltype(other_traits)>
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

        template<typename Ptr, special_mem_req OtherReq>
        constexpr void assign_ptr(
            const Ptr& other_ptr,
            const other_mem_traits<OtherReq>& other_traits
        )
            requires requires //
        {
            assign_traits(other_traits);
            this->traits_.construct(this->allocator_, get_ptr(), other_ptr);
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
            if(other.has_value()) traits_.construct(get_ptr(), other.get_ptr());
        }

        template<special_mem_req OtherReq>
        constexpr void assign(const other_mem_traits<Req>& other)
            requires requires { assign_ptr(get_ptr(), other.traits_); }
        {
            const auto copy_assign = [ // clang-format off
                this,
                ptr = static_cast<const_pointer>(other.get_ptr()),
                other_traits = other.traits_ // clang-format on
            ](const Alloc&, const Alloc&, const constant<alloc_traits::after_assign> = {})
            {
                assign_ptr(ptr, other_traits); //
            };

            alloc_traits::assign(
                allocator_,
                other.allocator_,
                make_trivial_invocables(
                    [this](const Alloc& left, const Alloc& right, const constant<alloc_traits::before_assign>)
                    {
                        if(left != right) reset();
                    },
                    copy_assign,
                    copy_assign
                )
            );
        }

        struct move_assign_op
        {
            basic_object_allocation& this_;
            basic_object_allocation&& other;

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
        constexpr void assign(other_mem_traits<Req>&& other) noexcept( //
            noexcept( //
                alloc_traits::assign(
                    allocator_,
                    ::std::move(other.allocator_),
                    move_assign_op{*this, ::std::move(other)}
                )
            )
        )
            requires requires //
        {
            alloc_traits::assign(
                this->allocator_,
                ::std::move(other.allocator_),
                move_assign_op{*this, ::std::move(other)}
            ); //
        }
        {
            if(this == &other) return *this;

            alloc_traits::assign(
                allocator_,
                ::std::move(other.allocator_),
                move_assign_op{*this, ::std::move(other)}
            );

            return *this;
        }

        template<special_mem_req OtherReq>
        constexpr void swap_ptr(
            Alloc& other_alloc,
            const pointer& other_ptr,
            const other_mem_traits<OtherReq>& other_traits
        )
            requires requires //
        {
            swap_traits(other_traits);
            this->traits_.construct(this->allocator_, get_ptr(), other_ptr);
        }
        {
            if(type() == other_traits.type())
            {
                if constexpr(requires { traits_.do_swap(get_ptr(), other_ptr); })
                {
                    traits_.do_swap(get_ptr(), other_ptr);
                    return;
                }
            }
            else
            {
                const auto size = other_traits.size();


                destroy();

                if(allocated_.size < size)
                {
                    deallocate();
                    allocated_ = alloc_traits::get_allocated(allocator_, size);
                }

                traits_ = other_traits;
            }

            traits_.construct(allocator_, get_ptr(), other_ptr);
        }

        struct swap_op
        {
            basic_object_allocation& this_;
            basic_object_allocation& other;

            constexpr void swap_allocated(basic_object_allocation& other) noexcept
            {
                ::std::swap(this_.allocated_, other.allocated_);
                ::std::swap(this_.traits_, other.traits_);
            };

            constexpr void
                operator()(const Alloc&, const Alloc&, const constant<alloc_traits::before_swap>) //
                noexcept(noexcept(this_.reset()))
            {
                this_.reset(); //
            }

            constexpr void
                operator()(const Alloc&, const Alloc&, const constant<alloc_traits::after_swap>) noexcept
            {
                swap_allocated(other); //
            }

            constexpr void operator()(const Alloc& left, const Alloc& right)
                requires requires { this_.swap_ptr(other.get_ptr(), other.traits_); }
            {
                if(left == right)
                {
                    this_.reset();
                    swap_allocated();
                }
                else this_.swap_ptr(other.get_ptr(), other.traits_);
            }
        };

    public:
        basic_object_allocation() = default;

        constexpr basic_object_allocation(const Alloc& alloc) noexcept: allocator_(alloc) {}

        template<
            typename... Args,
            ::std::constructible_from<Args...> T,
            typename ValueType = ::std::decay_t<T>,
            typename IdentityT = ::std::type_identity<ValueType> // clang-format off
        > // clang-format on
            requires ::std::constructible_from<mem_traits, IdentityT>
        constexpr basic_object_allocation(
            const ::std::in_place_type_t<T>,
            Args&&... args,
            const Alloc& alloc = Alloc{}
        ):
            basic_object_allocation(alloc),
            allocated_(alloc_traits::get_allocated(allocator_, sizeof(ValueType))),
            traits_(IdentityT{})
        {
            alloc_traits::construct(allocator_, get_ptr(), ::std::forward<Args>(args)...);
        }

        template<special_mem_req OtherReq>
        constexpr basic_object_allocation(const basic_object_allocation<OtherReq, Alloc>& other)
            requires ::std::
                constructible_from<basic_object_allocation, private_tag, decltype(other)>
            : basic_object_allocation(private_tag{}, other)
        {
        }

        template<special_mem_req OtherReq>
        constexpr basic_object_allocation(basic_object_allocation<OtherReq, Alloc>&& other) noexcept
            requires ::std::constructible_from<mem_traits, decltype(other.traits_)>
            :
            allocator_(::std::move(other.allocator_)),
            allocated_(other.allocated_),
            traits_(other.traits_)
        {
        }

        constexpr basic_object_allocation(const basic_object_allocation& other)
            requires ::std::
                constructible_from<basic_object_allocation, private_tag, decltype(other)>
            : basic_object_allocation(private_tag{}, other)
        {
        }

        basic_object_allocation(basic_object_allocation&&) noexcept = default;

        constexpr basic_object_allocation& operator=(const basic_object_allocation& other)
            requires requires { assign(other); }
        {
            if(this == &other) return *this;
            assign(other);
            return *this;
        }

        template<special_mem_req OtherReq>
        constexpr basic_object_allocation&
            operator=(const basic_object_allocation<OtherReq, Alloc>& other)
            requires requires { assign(other); }
        {
            assign(other);
            return *this;
        }

        constexpr basic_object_allocation& operator=(basic_object_allocation&& other) //
            noexcept(noexcept(assign(::std::move(other))))
            requires requires { assign(::std::move(other)); }
        {
            if(this == &other) return *this;
            assign(::std::move(other));
            return *this;
        }

        template<special_mem_req OtherReq>
        constexpr basic_object_allocation&
            operator=(basic_object_allocation<OtherReq, Alloc>&& other) //
            noexcept(noexcept(assign(::std::move(other))))
            requires requires { assign(::std::move(other)); }
        {
            assign(::std::move(other));
            return *this;
        }

        constexpr ~basic_object_allocation() noexcept(noexcept(destroy())) { reset(); }

        [[nodiscard]] constexpr auto get_allocator() const noexcept { return allocator_; }

        [[nodiscard]] constexpr auto type() const noexcept { return traits_.type(); }

        [[nodiscard]] constexpr auto size() const noexcept { return traits_.size(); }

        [[nodiscard]] constexpr auto reserved() const noexcept { return allocated_.size; }

        template<typename T>
        [[nodiscard]] constexpr auto& get() noexcept
        {
            return dereference_to<T>(get_ptr());
        }

        template<typename T>
        [[nodiscard]] constexpr auto& get() const noexcept
        {
            return dereference_to<const T>(get_ptr());
        }

        constexpr void reset() noexcept(noexcept(destroy()))
        {
            destroy();
            deallocate();
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return traits_.has_value(); }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }

        template<typename T, typename... Args>
        constexpr ::std::decay_t<T>& emplace(Args&&... args)
            requires ::std::constructible_from<::std::decay_t<T>, Args...> &&
            requires { this->assign_traits(special_mem_req::for_type<T>); }
        {
            if constexpr(move_assignable<T>)
                if(type_id<T>() == type()) { return (get<T>() = T{::std::forward<Args>(args)...}); }

            assign_traits(special_mem_req::for_type<T>);

            alloc_traits::construct(
                allocator_,
                get<::std::decay_t<T>>(),
                ::std::forward<Args>(args)...
            );

            return get<T>();
        }

        template<typename ValueType, typename U, typename... Args>
        constexpr auto& emplace(const std::initializer_list<U> il, Args&&... args)
            requires requires { this->emplace<ValueType>(il, ::std::declval<Args>()...); }
        {
            return emplace<ValueType>(il, ::std::forward<Args>(args)...);
        }

        template<special_mem_req OtherReq>
        constexpr void swap(basic_object_allocation<OtherReq, Alloc>& other) noexcept(
            noexcept(alloc_traits::swap(this->allocator_, other.allocator_, swap_op{*this, other}))
        )
            requires requires //
        {
            alloc_traits::swap(this->allocator_, other.allocator_, swap_op{*this, other});
        }
        {
            alloc_traits::swap(allocator_, other.allocator_, swap_op{*this, other});
        }

    private:
        Alloc allocator_{};
        typename alloc_traits::allocated allocated_{};
        mem_traits traits_{};
    };
}