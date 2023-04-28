#pragma once

#include "allocator_aware.h"
#include "../utility/implementation_reference.h"
#include "../compilation_config_in.h"

namespace stdsharp
{
    template<special_mem_req, typename>
    class basic_object_allocation;

    namespace details
    {
        template<expr_req Req, typename... Args>
        using special_mem_reference = implementation_reference<Req, void, Args...>;

        template<special_mem_req Req, special_mem_req Other>
        static constexpr bool req_compatible = Req <= Other;

        template<typename T, typename Allocator, special_mem_req Req>
        concept req_compatible_for =
            req_compatible<Req, allocator_traits<Allocator>::template mem_req_for<T>>;

        template<special_mem_req Req, typename Alloc>
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

                constexpr void do_swap(ptr_cref left, ptr_cref right) const
                    noexcept(swp_req == expr_req::no_exception)
                    requires(swp_req >= expr_req::well_formed)
                {
                    stdsharp::get<5>(to_base())(left, right); // NOLINT(*-magic-numbers)
                }

                [[nodiscard]] constexpr auto type() const noexcept { return current_type_; }

                [[nodiscard]] constexpr auto size() const noexcept { return type_size_; }

                [[nodiscard]] constexpr auto has_value() const noexcept { return size() != 0; }

                constexpr operator bool() const noexcept { return has_value(); }

            private:
                ::std::string_view current_type_ = type_id<void>;
                ::std::size_t type_size_{};
            };
        };

        template<special_mem_req Req, typename Alloc>
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

        template<special_mem_req Req, allocator_req Alloc>
        class STDSHARP_EBO basic_object_allocation : public allocator_aware_traits<Alloc>, Alloc
        {
            using this_t = basic_object_allocation;

            using m_base = allocator_aware_traits<Alloc>;

            using alloc_traits = typename m_base::traits;

            using typename m_base::allocator_type;

            constexpr allocator_type get_allocator() const noexcept { return *this; }

            template<special_mem_req OtherReq>
            using other_this_t = basic_object_allocation<OtherReq, allocator_type>;

        public:
            using typename alloc_traits::pointer;
            using typename alloc_traits::const_pointer;
            using typename alloc_traits::size_type;
            using alloc_construct_fn = typename alloc_traits::construct_fn;
            using typename alloc_traits::allocation;
            using typename alloc_traits::allocation_info;

            using allocation_ref = const allocation&;

            using mem_traits = special_mem_traits<Req, Alloc>;

            indexed_values<allocator_type, mem_traits> compressed_{};
            allocation allocation_{};

            constexpr allocator_type& get_allocator() noexcept
            {
                return stdsharp::get<0>(compressed_);
            }

            constexpr mem_traits& get_mem_traits() noexcept
            {
                return stdsharp::get<1>(compressed_);
            }

            constexpr auto& get_allocation() noexcept { return allocation_; }

            constexpr allocation_info get_allocation_info() const noexcept
            {
                return {get_allocation(), compressed_.size()};
            }

        public:
            template<
                typename... Args,
                typename T,
                req_compatible_for<allocator_type, Req> ValueType =
                    ::std::decay_t<T> // clang-format off
            > // clang-format on
                requires(alloc_traits::template construct_req<ValueType, Args...> >=
                         expr_req::well_formed)
            constexpr basic_object_allocation(
                const ::std::allocator_arg_t,
                const allocator_type& alloc,
                const ::std::in_place_type_t<T>,
                Args&&... args
            ):
                compressed_(alloc, ::std::type_identity<ValueType>{}),
                allocation_( //
                    make_allocation_by_obj<T>(
                        get_allocator(),
                        ptr_cast<T>(),
                        ::std::forward<Args>(args)...
                    )
                )
            {
            }

            template<special_mem_req OtherReq>
                requires req_compatible<Req, OtherReq>
            constexpr basic_object_allocation(
                const basic_object_allocation<OtherReq, allocator_type>& other,
                const allocator_type& alloc
            ):
                compressed_(alloc, other.get_mem_traits()),
                allocation_( //
                    m_base::copy_construct(
                        get_allocator(),
                        other.get_allocation(),
                        [&traits = get_mem_traits()](auto& alloc, auto& dest, auto& src)
                        {
                            traits.construct(alloc, dest.begin(), src.cbegin()); //
                        }
                    )
                )
            {
            }

            basic_object_allocation(const this_t&)
                requires false;

            template<special_mem_req OtherReq>
                requires req_compatible<Req, OtherReq>
            constexpr basic_object_allocation(
                basic_object_allocation<OtherReq, allocator_type>&& other,
                const allocator_type& alloc
            ) noexcept:
                compressed_(alloc, other.get_mem_traits()),
                allocation_(m_base::move_construct(other.get_allocation()))
            {
            }

            basic_object_allocation(this_t&&)
                requires false;

        private:
            template<special_mem_req OtherReq>
            struct copy_assign_fn
            {
                allocator_type& alloc;
                const mem_traits& traits;
                const special_mem_traits<OtherReq, Alloc>& other_traits;

                constexpr void
                    operator()(allocator_type& alloc, allocation_ref allocation) const noexcept
                {
                    if(allocation) traits.destruct(alloc, allocation.begin());
                }

                constexpr void
                    operator()(allocator_type& alloc, allocation_ref dest, allocation_ref src) const
                    noexcept(Req.copy_construct >= expr_req::no_exception)
                    requires(Req.copy_construct >= expr_req::well_formed)
                {
                    (*this)(alloc, dest);
                    other_traits.construct(alloc, dest.begin(), src.cbegin());
                }

                constexpr void operator()(allocation_ref dest, allocation_ref src) const noexcept(
                    Req.copy_assign >= expr_req::no_exception &&
                    Req.copy_construct >= expr_req::no_exception
                )
                    requires(
                        Req.copy_assign >= expr_req::well_formed &&
                        Req.copy_construct >= expr_req::well_formed
                    )
                {
                    if(traits.type() == other_traits.type())
                        traits.assign(dest.begin(), src.cbegin());
                    else (*this)(alloc, dest, src);
                }
            };

            template<special_mem_req OtherReq>
            struct move_assign_fn
            {
                allocator_type& alloc;
                const mem_traits& traits;
                const special_mem_traits<OtherReq, Alloc>& other_traits;

                constexpr void
                    operator()(allocator_type& alloc, allocation_ref allocation) const noexcept
                {
                    if(allocation) traits.destruct(alloc, allocation.begin());
                }

                constexpr void
                    operator()(allocator_type& alloc, allocation_ref dest, allocation_ref src) const
                    noexcept(Req.move_construct >= expr_req::no_exception)
                    requires(Req.move_construct >= expr_req::well_formed)
                {
                    (*this)(alloc, dest);
                    other_traits.construct(alloc, dest.begin(), src.cbegin());
                }

                constexpr void operator()(allocation_ref dest, allocation_ref src) const
                    noexcept(Req.move_assign >= expr_req::no_exception)
                    requires(Req.move_assign >= expr_req::well_formed)
                {
                    if(traits.type() == other_traits.type())
                        traits.assign(dest.begin(), src.cbegin());
                    else (*this)(alloc, dest, src);
                }
            };

            template<special_mem_req OtherReq>
            static constexpr auto copy_assignable_test =
                m_base::template copy_assignable_test<copy_assign_fn<OtherReq>>;

            template<special_mem_req OtherReq>
            static constexpr auto move_assignable_test =
                m_base::template move_assignable_test<move_assign_fn<OtherReq>>;

        public:
            template<special_mem_req OtherReq>
                requires req_compatible<Req, OtherReq> &&
                (copy_assignable_test<OtherReq> >= expr_req::well_formed)
            constexpr this_t&
                operator=(const basic_object_allocation<OtherReq, allocator_type>& other)
            {
                if(this == &other) return *this;

                auto&& info = get_allocation_info();

                m_base::copy_assign(
                    info,
                    other.get_allocation_info(),
                    copy_assign_fn<OtherReq>{
                        get_allocator(),
                        get_mem_traits(),
                        other.get_mem_traits() //
                    }
                );

                allocation_ = info.allocation;
                get_mem_traits() = other.get_mem_traits();

                return *this;
            }

            template<special_mem_req OtherReq>
                requires req_compatible<Req, OtherReq> &&
                (move_assignable_test<OtherReq> >= expr_req::well_formed)
            constexpr this_t& operator=( //
                basic_object_allocation<OtherReq, allocator_type>&& other
            ) noexcept(move_assignable_test<OtherReq> >= expr_req::no_exception)
            {
                if(this == &other) return *this;

                auto&& info = get_allocation_info();
                auto&& other_info = other.get_allocation_info();

                m_base::move_assign(
                    info,
                    other_info,
                    move_assign_fn<OtherReq>{
                        get_allocator(),
                        get_mem_traits(),
                        other.get_mem_traits() //
                    }
                );

                allocation_ = info.allocation;
                get_mem_traits() = other.get_mem_traits();

                return *this;
            }

            constexpr void destroy(Alloc& alloc) noexcept
            {
                if(*this)
                {
                    compressed_.destruct(alloc, get_ptr());
                    compressed_ = {};
                }
            }

            constexpr void deallocate(Alloc& alloc) noexcept
            {
                if(allocation_.ptr == nullptr) return;

                alloc_traits::deallocate(alloc, get_ptr(), allocation_.size);
                allocation_ = {};
            }

            template<special_mem_req OtherReq>
            constexpr void assign_traits(
                Alloc& alloc,
                const details::special_mem_traits<OtherReq, Alloc>& other_traits
            )
            {
                if(type() == other_traits.type()) return;

                if(!other_traits) reset(alloc);

                const auto size = other_traits.size();

                destroy(alloc);

                if(allocation_.size < size)
                {
                    deallocate(alloc);
                    allocation_ = alloc_traits::get_allocated(alloc, size);
                }

                compressed_ = other_traits;
            }

            template<typename Ptr, special_mem_req OtherReq>
            constexpr void assign_ptr(
                Alloc& alloc,
                const Ptr& other_ptr,
                const details::special_mem_traits<OtherReq, Alloc>& other_traits
            )
            {
                if(!other_traits)
                {
                    if(*this) reset();
                    return;
                }

                if(type() == other_traits.type())
                {
                    if constexpr(requires { compressed_.assign(get_ptr(), other_ptr); })
                    {
                        compressed_.assign(get_ptr(), other_ptr);
                        return;
                    }
                }
                else assign_traits(other_traits);

                compressed_.construct(alloc, get_ptr(), other_ptr);
            }

            [[nodiscard]] constexpr auto& get_ptr() const noexcept { return allocation_.ptr; }

            [[nodiscard]] constexpr const_pointer& get_const_ptr() const noexcept
            {
                return allocation_.ptr;
            }

            template<typename T>
            [[nodiscard]] constexpr auto ptr_cast() const noexcept
            {
                return pointer_cast<T>(get_ptr());
            }

            basic_object_allocation() = default;
        };
    }

    template<special_mem_req Req, allocator_req Alloc>
        requires(Req.destruct == expr_req::no_exception)
    class basic_object_allocation<Req, Alloc> :
        basic_allocator_aware<Alloc, details::basic_object_allocation<Req, Alloc>>
    {
        using m_base = basic_allocator_aware<Alloc, details::basic_object_allocation<Req, Alloc>>;

        using m_base::allocator;
        using m_base::data;
        using alloc_construct_fn = typename allocator_traits<Alloc>::construct_fn;
        using mem_traits = typename m_base::data_t::mem_traits;

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

        template<typename T, typename... Args>
        static constexpr auto emplace_able =
            ::std::invocable<alloc_construct_fn, Alloc&, T*, Args...> &&
            details::req_compatible_for<T, Alloc, Req>;

    public:
        basic_object_allocation() = default;
        basic_object_allocation(const basic_object_allocation&) = default;
        basic_object_allocation(basic_object_allocation&&) noexcept = default;
        basic_object_allocation& operator=(const basic_object_allocation&) = default;
        basic_object_allocation& operator=(basic_object_allocation&&) noexcept = default;

        using m_base::m_base;
        using typename m_base::allocator_type;
        using allocator_traits = typename m_base::traits;
        using m_base::get_allocator;

        static constexpr auto req = Req;

        template<not_same_as<void> T, typename... Args>
            requires emplace_able<::std::decay_t<T>, Args...>
        constexpr decltype(auto) emplace(Alloc& alloc, Args&&... args)
        {
            using value_t = ::std::decay_t<T>;

            if(type_id<value_t> == type() &&
               try_same_type_emplace<value_t>(::std::forward<Args>(args)...))
                return get<value_t>();

            data.assign_traits(mem_traits{::std::type_identity<T>{}});

            allocator_traits::construct(alloc, ptr_cast<value_t>(), ::std::forward<Args>(args)...);

            return get<value_t>();
        }

        template<::std::same_as<void> = void>
        constexpr void emplace(Alloc& alloc) noexcept
        {
            destroy(alloc);
        }

        template<not_same_as<void> T, typename... Args, typename U>
        constexpr decltype(auto) emplace(
            Alloc& alloc,
            const ::std::initializer_list<U> il,
            Args&&... args //
        )
            requires emplace_able<::std::decay_t<T>, decltype(il), Args...>
        {
            return emplace<T, decltype(il), Args...>(alloc, il, ::std::forward<Args>(args)...);
        }

        template<not_same_as<void> T>
            requires emplace_able<::std::decay_t<T>, T>
        constexpr decltype(auto) emplace(Alloc& alloc, T&& t)
        {
            return emplace<::std::decay_t<T>, T>(alloc, ::std::forward<T>(t));
        }

        constexpr ~basic_object_allocation() noexcept { data.reset(allocator); }

        template<typename T>
        constexpr decltype(auto) get() noexcept
        {
            return data.template get<T>();
        }

        template<typename T>
        constexpr decltype(auto) get() const noexcept
        {
            return data.template get<T>();
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return data; }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return data; }

        [[nodiscard]] constexpr auto type() const noexcept { return data.type(); }

        [[nodiscard]] constexpr auto size() const noexcept { return data.size(); }

        [[nodiscard]] constexpr auto reserved() const noexcept { return data.reserved(); }
    };

    template<typename T, allocator_req Alloc>
    using object_allocation_like =
        basic_object_allocation<allocator_traits<Alloc>::template mem_req_for<T>, Alloc>;

    template<allocator_req Alloc>
    using trivial_object_allocation = basic_object_allocation<special_mem_req::trivial, Alloc>;

    template<allocator_req Alloc>
    using normal_movable_object_allocation =
        basic_object_allocation<special_mem_req::unique, Alloc>;

    template<allocator_req Alloc>
    using normal_object_allocation = basic_object_allocation<
        special_mem_req{
            .move_construct = expr_req::well_formed,
            .copy_construct = expr_req::well_formed,
            .copy_assign = expr_req::well_formed //
        },
        Alloc>;
}

#include "../compilation_config_out.h"