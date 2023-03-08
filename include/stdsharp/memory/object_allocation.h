#pragma once

#include "pointer_traits.h"
#include "allocator_traits.h"
#include "../type_traits/object.h"
#include "../type_traits/special_member.h"

namespace stdsharp
{
    namespace details
    {
        template<expr_req ExprReq, typename... Arg>
        struct special_mem_ptr
        {
            static constexpr auto is_noexcept = ExprReq == expr_req::no_exception;

            using ptr_t = void (*)(Arg&&...) noexcept(is_noexcept);

            ptr_t value;

            constexpr special_mem_ptr(const ptr_t ptr = nullptr) noexcept: value(ptr) {}

            constexpr void operator()(Arg&&... args) const noexcept(is_noexcept)
            {
                (*value)(::std::forward<Arg>(args)...);
            }
        };

        template<typename... Arg>
        struct special_mem_ptr<expr_req::ill_formed, Arg...>
        {
            static constexpr auto is_noexcept = false;

            special_mem_ptr(const auto) noexcept {}
        };

        template<special_mem_req Req, typename Ptr>
        struct mov_assign : special_mem_ptr<Req.move_assign, const Ptr&, const Ptr&>
        {
            template<typename T>
            static constexpr mov_assign for_type()
            {
                return [](const Ptr& left, const Ptr& right) //
                    noexcept(mov_assign::is_noexcept) //
                    requires(Req.move_assign >= expr_req::well_formed) //
                {
                    dereference_to<T>(left) = ::std::move(dereference_to<T>(right));
                };
            }
        };

        template<special_mem_req Req, typename Ptr, typename ConstPtr>
        struct cp_assign : special_mem_ptr<Req.copy_assign, const Ptr&, const ConstPtr&>
        {
            template<typename T>
            static constexpr cp_assign for_type()
            {
                return [](const Ptr& left, const ConstPtr& right) //
                    noexcept(cp_assign::is_noexcept) //
                    requires(Req.copy_assign >= expr_req::well_formed) //
                {
                    dereference_to<T>(left) = dereference_to<T>(right);
                };
            }
        };

        template<special_mem_req Req, typename Alloc>
        struct special_member_traits
        {
            using pointer = typename allocator_traits<Alloc>::pointer;
            using pointer_traits = pointer_traits<pointer>;
            using const_pointer = typename allocator_traits<Alloc>::const_pointer;

            struct mov_ctor :
                special_mem_ptr<Req.move_construct, Alloc&, const pointer&, const pointer&>
            {
                template<typename T>
                static constexpr mov_ctor for_type()
                {
                    return [](Alloc & alloc, const pointer& left, const pointer& right) //
                        noexcept(mov_ctor::is_noexcept) //
                        requires(Req.move_construct >= expr_req::well_formed) //
                    {
                        allocator_traits<Alloc>::construct(
                            alloc,
                            dereference_to<T>(left),
                            ::std::move(dereference_to<T>(right))
                        );
                    };
                }
            };

            struct cp_ctor :
                special_mem_ptr<Req.copy_construct, Alloc&, const pointer&, const const_pointer&>
            {
                template<typename T>
                static constexpr cp_ctor for_type()
                {
                    return [](Alloc & alloc, const pointer& left, const const_pointer& right) //
                        noexcept(cp_ctor::is_noexcept) //
                        requires(Req.copy_construct >= expr_req::well_formed) //
                    {
                        allocator_traits<Alloc>::construct(
                            alloc,
                            dereference_to<T>(left),
                            dereference_to<T>(right)
                        );
                    };
                }
            };

            struct dtor : special_mem_ptr<Req.destroy, Alloc&, const pointer&>
            {
                template<typename T>
                static constexpr dtor for_type()
                {
                    return [](Alloc & alloc, const pointer& ptr) //
                        noexcept(dtor::is_noexcept) //
                        requires(Req.destroy >= expr_req::well_formed) //
                    {
                        allocator_traits<Alloc>::destroy(alloc, dereference_to<T>(ptr));
                    };
                }
            };

            using mov_assign = mov_assign<Req, pointer>;

            using cp_assign = cp_assign<Req, pointer, const_pointer>;

            class impl : mov_ctor, cp_ctor, mov_assign, cp_assign, dtor
            {
                using alloc_traits = allocator_traits<Alloc>;

            public:
                template<typename T>
                    requires(Req <= special_mem_req::for_type<T>())
                static const impl for_type;

                impl() = default;

                template<typename T>
                    requires(Req <= special_mem_req::for_type<T>())
                constexpr impl(const ::std::type_identity<T>) noexcept:
                    current_type_(type_id<T>),
                    type_size_(sizeof(T)),
                    mov_ctor(mov_ctor::template for_type<T>()),
                    cp_ctor(cp_ctor::template for_type<T>()),
                    mov_assign(mov_assign::template for_type<T>()),
                    cp_assign(cp_assign::template for_type<T>()),
                    dtor(dtor::template for_type<T>())
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
                    noexcept(mov_ctor::is_noexcept)
                    requires(Req.move_construct >= expr_req::well_formed)
                {
                    static_cast<const mov_ctor&>(*this)(alloc, left, right);
                }

                constexpr void
                    construct(Alloc& alloc, const pointer& left, const const_pointer& right) const
                    noexcept(cp_ctor::is_noexcept)
                    requires(Req.copy_construct >= expr_req::well_formed)
                {
                    static_cast<const mov_ctor&>(*this)(alloc, left, right);
                }

                constexpr void assign(const pointer& left, const pointer& right) const
                    noexcept(mov_assign::is_noexcept)
                    requires(Req.move_assign >= expr_req::well_formed)
                {
                    static_cast<const mov_assign&>(*this)(left, right);
                }

                constexpr void assign(const pointer& left, const const_pointer& right) const
                    noexcept(cp_assign::is_noexcept)
                    requires(Req.copy_assign >= expr_req::well_formed)
                {
                    static_cast<const cp_assign&>(*this)(left, right);
                }

                constexpr void destruct(Alloc& alloc, const pointer& ptr) const
                    noexcept(dtor::is_noexcept)
                    requires(Req.destroy >= expr_req::well_formed)
                {
                    static_cast<const dtor&>(*this)(alloc, ptr);
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
    class basic_object_allocation
    {
        using alloc_traits = allocator_traits<Alloc>;
        using pointer = typename alloc_traits::pointer;
        using const_pointer = typename alloc_traits::const_pointer;
        using size_type = typename alloc_traits::size_type;
        using mem_traits = details::special_member_traits_t<Req, Alloc>;

        [[nodiscard]] constexpr auto& get_ptr() const noexcept { return allocated_.ptr; }

        constexpr void destroy() const noexcept(Req.destroy == expr_req::no_exception)
            requires(Req.destroy >= expr_req::well_formed)
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

        constexpr void reserve(const size_type size)
            requires requires { destroy(); }
        {
            if(!has_value())
            {
                allocated_ = {alloc_traits::allocate(allocator_, size), size};
                return;
            }

            destroy();

            if(allocated_.size >= size) return;

            deallocate();
            allocated_ = {
                alloc_traits::allocate(allocator_, size),
                size //
            };
        }

        template<typename Ptr, special_mem_req OtherReq>
        constexpr void assign_or_construct(
            const Ptr& other_ptr,
            const details::special_member_traits_t<OtherReq, Alloc>& other_traits
        )
            requires ::std::assignable_from<
                         mem_traits&,
                         const details::special_member_traits_t<OtherReq, Alloc>&> &&
            requires //
        {
            this->reset();
            this->traits_.construct(this->allocator_, get_ptr(), other_ptr);
        }
        {
            if(other_ptr == nullptr)
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
            else traits_ = other_traits;

            reserve(traits_.size());

            traits_.construct(allocator_, get_ptr(), other_ptr);
        }

        constexpr auto get_move_assign_op(basic_object_allocation&& other) //
            noexcept
        {
            struct fn
            {
                basic_object_allocation& this_;
                basic_object_allocation&& other;

                constexpr void copy_allocated(basic_object_allocation&& other) noexcept
                {
                    this_.allocated_ = other.allocated_;
                    this_.traits_ = other.traits_;
                };

                constexpr void operator()(
                    const Alloc&,
                    const Alloc&,
                    const constant<alloc_traits::before_assign> //
                ) noexcept(noexcept(this_.reset()))
                    requires requires { this_.reset(); }
                {
                    this_.reset(); //
                }

                constexpr void operator()(
                    const Alloc&,
                    const Alloc&,
                    const constant<alloc_traits::after_assign> //
                ) noexcept
                {
                    copy_allocated(other); //
                }

                constexpr void operator()(const Alloc& left, const Alloc& right)
                    requires requires //
                {
                    this_.reset();
                    this_.assign_or_construct(other.get_ptr(), other.traits_);
                }
                {
                    if(left == right)
                    {
                        this_.reset();
                        copy_allocated();
                    }
                    else this_.assign_or_construct(other.get_ptr(), other.traits_);
                }
            };

            return fn{*this, other};
        }

        struct private_tag
        {
        } private_tag_v;

        template<special_mem_req OtherReq>
            requires(Req.copy_construct == expr_req::well_formed) &&
                        ::std::constructible_from<
                            mem_traits,
                            details::special_member_traits_t<OtherReq, Alloc>>
        constexpr basic_object_allocation(
            const private_tag,
            const basic_object_allocation<OtherReq, Alloc>& other
        ):
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
        constexpr void assign_impl(const basic_object_allocation<OtherReq, Alloc>& other)
            requires requires { assign_or_construct(get_ptr(), other.traits_); }
        {
            const auto copy_assign = [this,
                                      ptr = static_cast<const_pointer>(other.get_ptr()),
                                      other_traits = other.traits_](
                                         const Alloc&,
                                         const Alloc&,
                                         const constant<alloc_traits::after_assign> = {}
                                     )
            {
                assign_or_construct(ptr, other_traits); //
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


    public:
        template<typename T, typename AllocArg>
            requires ::std::constructible_from<Alloc, AllocArg> &&
                         (Req.move_construct == expr_req::well_formed) &&
                         requires { mem_traits::template for_type<::std::decay_t<T>>; }
        constexpr basic_object_allocation(T&& value, AllocArg&& alloc_arg):
            allocator_(::std::forward<AllocArg>(alloc_arg)),
            allocated_(alloc_traits::get_allocated(allocator_, sizeof(::std::decay_t<T>))),
            traits_(mem_traits::template for_type<::std::decay_t<T>>)
        {
            traits_.move_construct(allocator_, get_ptr(), ::std::forward<T>(value));
        }

        template<typename AllocArg>
            requires ::std::constructible_from<Alloc, AllocArg>
        constexpr basic_object_allocation(AllocArg&& alloc_arg) //
            noexcept(nothrow_constructible_from<Alloc, AllocArg>):
            allocator_(::std::forward<AllocArg>(alloc_arg))
        {
        }

        basic_object_allocation() = default;

        constexpr basic_object_allocation(const basic_object_allocation& other)
            requires ::std::constructible_from<
                basic_object_allocation,
                private_tag,
                const basic_object_allocation& // clang-format off
            > // clang-format on
            : basic_object_allocation(private_tag_v, other)
        {
        }

        template<special_mem_req OtherReq>
        constexpr basic_object_allocation(const basic_object_allocation<OtherReq, Alloc>& other)
            requires ::std::constructible_from<
                basic_object_allocation,
                private_tag,
                const basic_object_allocation<OtherReq, Alloc>& // clang-format off
            > // clang-format on
            : basic_object_allocation(private_tag_v, other)
        {
        }

        basic_object_allocation(basic_object_allocation&&) noexcept = default;

        template<special_mem_req OtherReq>
            requires ::std::constructible_from<
                         mem_traits,
                         details::special_member_traits_t<OtherReq, Alloc>>
        constexpr basic_object_allocation(basic_object_allocation<OtherReq, Alloc>&& other):
            allocator_(::std::move(other.allocator_)),
            allocated_(other.allocated_),
            traits_(other.traits_)
        {
        }

        constexpr basic_object_allocation& operator=(const basic_object_allocation& other)
            requires requires { assign_or_construct(get_ptr(), this->traits_); }
        {
            if(this == &other) return *this;

            const auto copy_assign = [this,
                                      ptr = static_cast<const_pointer>(other.get_ptr()),
                                      other_traits = other.traits_](
                                         const Alloc&,
                                         const Alloc&,
                                         const constant<alloc_traits::after_assign> = {}
                                     )
            {
                assign_or_construct(ptr, other_traits); //
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

            return *this;
        }

        constexpr basic_object_allocation& operator=(basic_object_allocation&& other) //
            noexcept( //
                noexcept( //
                    alloc_traits::assign(
                        allocator_,
                        ::std::move(other.allocator_),
                        get_move_assign_op(other.allocator_)
                    )
                )
            )
            requires requires //
        {
            alloc_traits::assign(
                this->allocator_,
                ::std::move(other.allocator_),
                get_move_assign_op(other.allocator_)
            ); //
        }
        {
            if(this == &other) return *this;

            alloc_traits::assign(allocator_, ::std::move(other.allocator_), get_move_assign_op());

            return *this;
        }

        constexpr ~basic_object_allocation() noexcept(noexcept(destroy()))
            requires requires { destroy(); }
        {
            reset();
        }

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
            requires requires { destroy(); }
        {
            destroy();
            deallocate();
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return traits_.empty(); }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }

        template<typename T, typename... Args>
            requires(Req <= special_mem_req::for_type<T>) && ::std::constructible_from<T, Args...>
        constexpr void emplace(Args&&... args)
        {
            if(type_id<T>() == type())
            {
                get<T>() = T{::std::forward<Args>(args)...};
                return;
            }

            reserve(sizeof(T));

            traits_ = special_mem_req::for_type<T>;

            alloc_traits::construct(
                allocator_,
                dereference_to<T>(get_ptr()),
                ::std::forward<Args>(args)...
            );
        }

    private:
        Alloc allocator_{};
        typename alloc_traits::allocated allocated_{};
        mem_traits traits_{};
    };
}