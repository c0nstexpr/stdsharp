#pragma once

#include "pointer_traits.h"
#include "allocator_aware.h"
#include "../utility/implementation_reference.h"
#include "../cassert/cassert.h"

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
        class basic_object_allocation : allocator_aware_traits<Alloc>
        {
            using m_base = allocator_aware_traits<Alloc>;

            using alloc_traits = typename m_base::traits;
            using pointer = typename alloc_traits::pointer;
            using const_pointer = typename alloc_traits::const_pointer;
            using size_type = typename alloc_traits::size_type;
            using alloc_construct_fn = typename alloc_traits::construct_fn;
            using allocated = typename alloc_traits::allocation;

            using ptr_cref = const pointer&;
            using cptr_cref = const const_pointer&;

            using mem_traits = typename details::special_mem_traits<Req, Alloc>;

            allocated allocated_{};
            mem_traits traits_{};

            template<special_mem_req OtherReq>
                requires details::req_compatible<Req, OtherReq>
            constexpr basic_object_allocation(
                const allocated& allocated,
                const details::special_mem_traits<OtherReq, Alloc>& other
            ) noexcept:
                allocated_(allocated), traits_(other)
            {
            }

            template<special_mem_req OtherReq>
                requires details::req_compatible<Req, OtherReq>
            constexpr basic_object_allocation(
                Alloc& alloc,
                const ::std::size_t size,
                const details::special_mem_traits<OtherReq, Alloc>& other
            ):
                basic_object_allocation(alloc_traits::get_allocated(alloc, size), other)
            {
            }

            constexpr void destroy(Alloc& alloc) noexcept
            {
                if(*this)
                {
                    traits_.destruct(alloc, get_ptr());
                    traits_ = {};
                }
            }

            constexpr void deallocate(Alloc& alloc) noexcept
            {
                if(allocated_.ptr == nullptr) return;

                alloc_traits::deallocate(alloc, get_ptr(), allocated_.size);
                allocated_ = {};
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

                if(allocated_.size < size)
                {
                    deallocate(alloc);
                    allocated_ = alloc_traits::get_allocated(alloc, size);
                }

                traits_ = other_traits;
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
                    if constexpr(requires { traits_.assign(get_ptr(), other_ptr); })
                    {
                        traits_.assign(get_ptr(), other_ptr);
                        return;
                    }
                }
                else assign_traits(other_traits);

                traits_.construct(alloc, get_ptr(), other_ptr);
            }

            [[nodiscard]] constexpr auto& get_ptr() const noexcept { return allocated_.ptr; }

            [[nodiscard]] constexpr const_pointer& get_const_ptr() const noexcept
            {
                return allocated_.ptr;
            }

            template<typename T>
            [[nodiscard]] constexpr auto ptr_cast() const noexcept
            {
                return pointer_cast<T>(get_ptr());
            }

        public:
            template<
                typename... Args,
                typename T,
                typename ValueType = ::std::decay_t<T>,
                typename IdentityT = ::std::type_identity<ValueType> // clang-format off
            > // clang-format on
                requires ::std::constructible_from<mem_traits, IdentityT> &&
                ::std::invocable<alloc_construct_fn, Alloc&, ValueType*, Args...>
            constexpr basic_object_allocation(
                Alloc& alloc,
                const ::std::in_place_type_t<T>,
                Args&&... args
            ):
                basic_object_allocation(sizeof(ValueType), mem_traits{IdentityT{}})
            {
                alloc_traits::construct(alloc, ptr_cast<T>(), ::std::forward<Args>(args)...);
            }

            template<special_mem_req OtherReq>
                requires details::req_compatible<Req, OtherReq> &&
                (Req.copy_construct == expr_req::well_formed)
            constexpr basic_object_allocation(
                Alloc& alloc,
                const basic_object_allocation<OtherReq, Alloc>& other
            ):
                basic_object_allocation(alloc, other.size(), other.traits_)
            {
                if(other.has_value()) traits_.construct(alloc, get_ptr(), other.get_const_ptr());
            }

        private:
            template<special_mem_req OtherReq>
            static constexpr allocated move_allocated(
                Alloc& alloc,
                const Alloc& other_alloc,
                const basic_object_allocation<OtherReq, Alloc>& other
            )
            {
                if(other)
                {
                    if(alloc == other_alloc) return other.allocated_;
                    const auto& allocated = alloc_traits::get_allocation(alloc, other.size());
                    other.traits_.construct(alloc, allocated.ptr, other.ptr);

                    return allocated;
                }
                return {};
            }

        public:
            template<special_mem_req OtherReq>
                requires details::req_compatible<Req, OtherReq> &&
                             (Req.move_construct == expr_req::well_formed)
            constexpr basic_object_allocation(
                Alloc& alloc,
                const Alloc& other_alloc,
                basic_object_allocation<OtherReq, Alloc>&& other
            ) noexcept(Req.move_construct == expr_req::no_exception):
                allocated_(move_allocated(alloc, other_alloc, other)), traits_(other.traits_)
            {
            }

            template<special_mem_req OtherReq>
                requires details::req_compatible<Req, OtherReq>
            constexpr basic_object_allocation(basic_object_allocation<OtherReq, Alloc>&& other) //
                noexcept(Req.move_construct == expr_req::no_exception):
                allocated_(other.allocated_), traits_(other.traits_)
            {
            }

        private:
            template<bool IsCopy, special_mem_req OtherReq>
            static constexpr bool assignable_from_ptr =
                details::req_compatible<Req, OtherReq> &&
                requires( //
                    Alloc alloc,
                    pointer ptr,
                    ::std::conditional_t<IsCopy, const_pointer, pointer> other_ptr
                ) //
            {
                traits_.construct(alloc, ptr, other_ptr); //
            };

            using enum allocator_assign_operation;

            template<special_mem_req OtherReq, allocator_assign_operation op>
                requires(op == before_assign)
            constexpr void assign_allocator(
                const basic_object_allocation<OtherReq, Alloc>&,
                Alloc& left,
                const Alloc& alloc_right
            ) noexcept
            {
                if(left != alloc_right) reset(left);
            }

            template<special_mem_req OtherReq, allocator_assign_operation op = after_assign>
                requires(op == after_assign) && assignable_from_ptr<true, OtherReq>
            constexpr void
                assign_allocator(const basic_object_allocation<OtherReq, Alloc>& other, Alloc& left, const Alloc&)
            {
                assign_ptr(left, other.get_const_ptr(), other.traits);
            }

            template<special_mem_req OtherReq>
                requires details::req_compatible<Req, OtherReq>
            constexpr void copy_allocated(const basic_object_allocation<OtherReq, Alloc>& other) //
                noexcept
            {
                allocated_ = other.allocated_;
                traits_ = other.traits_;
            };

            template<special_mem_req OtherReq, allocator_assign_operation op>
                requires(op == before_assign)
            constexpr void
                assign_allocator(basic_object_allocation<OtherReq, Alloc>&&, Alloc& left, const Alloc&) //
                noexcept
            {
                reset(left);
            }

            template<special_mem_req OtherReq, allocator_assign_operation op>
                requires(op == after_assign) && details::req_compatible<Req, OtherReq>
            constexpr void
                assign_allocator(basic_object_allocation<OtherReq, Alloc>&& other, const Alloc&, const Alloc&) noexcept
            {
                copy_allocated(other);
            }

            template<special_mem_req OtherReq>
            constexpr void assign_allocator(
                basic_object_allocation<OtherReq, Alloc>&& other,
                Alloc& left,
                const Alloc& right
            )
                requires assignable_from_ptr<false, OtherReq>
            {
                if(left == right)
                {
                    reset(left);
                    copy_allocated(other);
                }
                else assign_ptr(left, other.get_ptr(), other.traits_);
            }

            using enum allocator_swap_operation;

            constexpr void swap_allocated(basic_object_allocation& other) noexcept
            {
                ::std::swap(allocated_, other.allocated_);
                ::std::swap(traits_, other.traits_);
            }

            template<allocator_swap_operation op>
            constexpr void
                swap_allocator(basic_object_allocation& other, const Alloc&, const Alloc&) noexcept
            {
                if constexpr(op == after_swap) swap_allocated(other);
            }

            constexpr void swap_allocator(
                basic_object_allocation& other,
                const Alloc& left,
                const Alloc& right
            ) noexcept(alloc_traits::is_always_equal::value || !is_debug)
            {
                if(left == right) swap_allocated(other);
                if constexpr(is_debug)
                    throw ::std::invalid_argument{"Swap with different allocators"};
            }

        public:
            using allocator_type = Alloc;

            static constexpr auto req = Req;

            basic_object_allocation() = default;

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

            constexpr void reset(Alloc& alloc) noexcept
            {
                destroy(alloc);
                deallocate(alloc);
            }

            [[nodiscard]] constexpr bool has_value() const noexcept { return traits_; }

            [[nodiscard]] constexpr explicit operator bool() const noexcept { return traits_; }
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