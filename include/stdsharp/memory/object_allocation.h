#pragma once

#include "allocator_aware.h"
#include "../utility/implementation_reference.h"

namespace stdsharp
{
    template<special_mem_req, typename>
    class basic_object_allocation;

    namespace details
    {
        template<typename T, typename Allocator, special_mem_req Req>
        concept req_compatible_for =
            Req <= allocator_aware_traits<Allocator>::template allocation_for<T>::mem_req;

        template<
            special_mem_req Req,
            typename Alloc,
            typename Traits = allocator_aware_traits<Alloc>>
        struct allocation_dispatcher_traits : allocator_aware_traits<Alloc>
        {
            using alloc = Traits::allocator_type;
            using alloc_cref = const alloc&;
            using typename Traits::allocation;
            using allocation_cref = const allocation&;

            static constexpr auto mov_ctor_req = Req.move_construct;
            static constexpr auto cp_ctor_req = Req.copy_construct;
            static constexpr auto mov_assign_req = Req.move_assign;
            static constexpr auto cp_assign_req = Req.copy_assign;
            static constexpr auto dtor_req = Req.destruct;
            static constexpr auto swp_req = Req.swap;

            template<expr_req ExprReq, typename... Args>
            using ctor_dispatcher = implement_dispatcher<ExprReq, allocation, alloc&, Args...>;

            template<expr_req ExprReq, typename... Args>
            using write_dispatcher =
                implement_dispatcher<ExprReq, void, alloc&, allocation&, Args...>;

            using dispatchers = stdsharp::indexed_values<
                ctor_dispatcher<mov_ctor_req, allocation&>,
                ctor_dispatcher<cp_ctor_req, allocation_cref>,
                write_dispatcher<mov_assign_req, alloc&, allocation&>,
                write_dispatcher<cp_assign_req, alloc_cref, allocation_cref>,
                write_dispatcher<dtor_req>,
                write_dispatcher<swp_req, alloc&, allocation&> // clang-format off
            >; // clang-format on
        };

        template<special_mem_req Req, typename Alloc>
        class allocation_dispatchers : allocation_dispatcher_traits<Req, Alloc>
        {
            using traits = allocation_dispatcher_traits<Req, Alloc>;
            using typename traits::dispatchers;
            using typename traits::alloc_traits;
            using typename traits::alloc;
            using typename traits::alloc_cref;
            using typename traits::allocation;
            using typename traits::allocation_cref;

            using traits::mov_ctor_req;
            using traits::cp_ctor_req;
            using traits::mov_assign_req;
            using traits::cp_assign_req;
            using traits::dtor_req;
            using traits::swp_req;

        protected:
            constexpr allocation_dispatchers(
                const dispatchers& b,
                const ::std::string_view current_type,
                const ::std::size_t type_size
            ) noexcept:
                dispatchers_(b), current_type_(current_type), type_size_(type_size)
            {
            }

        public:
            static constexpr auto req = Req;

            allocation_dispatchers() = default;

            template<req_compatible_for<Alloc, Req> T>
            constexpr allocation_dispatchers(const ::std::type_identity<T>) noexcept:
                allocation_dispatchers(
                    dispatchers(
                        [](alloc & alloc, allocation & other) //
                        noexcept(mov_ctor_req == expr_req::no_exception) //
                        requires(mov_ctor_req >= expr_req::well_formed) //
                        {
                            return traits::template move_construct<T>(alloc, other); //
                        },
                        [](alloc & alloc, allocation_cref other) //
                        noexcept(cp_ctor_req == expr_req::no_exception) //
                        requires(cp_ctor_req >= expr_req::well_formed) //
                        {
                            return traits::template copy_construct<T>(alloc, other); //
                        },
                        []( //
                            alloc & dst_alloc,
                            allocation & dst_allocation,
                            alloc & src_alloc,
                            allocation & src_allocation
                        ) noexcept(mov_assign_req == expr_req::no_exception) //
                        requires(mov_assign_req >= expr_req::well_formed) //
                        {
                            return traits::template move_assign<T>(
                                dst_alloc,
                                dst_allocation,
                                src_alloc,
                                src_allocation
                            ); //
                        },
                        []( //
                            alloc & dst_alloc,
                            allocation & dst_allocation,
                            alloc_cref src_alloc,
                            allocation_cref src_allocation //
                        ) noexcept(cp_assign_req == expr_req::no_exception) //
                        requires(cp_assign_req >= expr_req::well_formed) //
                        {
                            return traits::template copy_assign<T>(
                                dst_alloc,
                                dst_allocation,
                                src_alloc,
                                src_allocation
                            ); //
                        },
                        [](alloc & alloc, allocation & allocation) //
                        noexcept(dtor_req == expr_req::no_exception) //
                        requires(dtor_req >= expr_req::well_formed) //
                        {
                            return traits::template destroy<T>(alloc, allocation); //
                        },
                        []( //
                            alloc & dst_alloc,
                            allocation & dst_allocation,
                            alloc & src_alloc,
                            allocation & src_allocation
                        ) //
                        noexcept(swp_req == expr_req::no_exception) //
                        requires(swp_req >= expr_req::well_formed) //
                        {
                            return traits::template swap<T>(
                                dst_alloc,
                                dst_allocation,
                                src_alloc,
                                src_allocation
                            ); //
                        }
                    ),
                    type_id<T>,
                    sizeof(T)
                )
            {
            }

            template<special_mem_req OtherReq>
            constexpr allocation_dispatchers( //
                const allocation_dispatchers<OtherReq, Alloc>& other
            ) noexcept:
                allocation_dispatchers(other.dispatchers_, other.current_type_, other.type_size_)
            {
            }

            [[nodiscard]] constexpr auto construct(alloc& alloc, allocation& allocation) const
                noexcept(mov_ctor_req == expr_req::no_exception)
                requires(mov_ctor_req >= expr_req::well_formed)
            {
                return get<0>(dispatchers_)(alloc, allocation);
            }

            [[nodiscard]] constexpr auto construct(alloc& alloc, allocation_cref allocation) const
                noexcept(cp_ctor_req == expr_req::no_exception)
                requires(cp_ctor_req >= expr_req::well_formed)
            {
                return get<1>(dispatchers_)(alloc, allocation);
            }

            constexpr void assign(
                alloc& dst_alloc,
                allocation& dst_allocation,
                alloc& src_alloc,
                allocation& src_allocation
            ) const noexcept(mov_assign_req == expr_req::no_exception)
                requires(mov_assign_req >= expr_req::well_formed)
            {
                get<2>(dispatchers_)(dst_alloc, dst_allocation, src_alloc, src_allocation);
            }

            constexpr void assign(
                alloc& dst_alloc,
                allocation& dst_allocation,
                alloc_cref src_alloc,
                allocation_cref src_allocation
            ) const noexcept(cp_assign_req == expr_req::no_exception)
                requires(cp_assign_req >= expr_req::well_formed)
            {
                get<3>(dispatchers_)(dst_alloc, dst_allocation, src_alloc, src_allocation);
            }

            constexpr void destroy(alloc& alloc, allocation& allocation) const
                noexcept(dtor_req == expr_req::no_exception)
                requires(dtor_req >= expr_req::well_formed)
            {
                get<4>(dispatchers_)(alloc, allocation);
            }

            constexpr void swap(
                alloc& dst_alloc,
                allocation& dst_allocation,
                alloc& src_alloc,
                allocation& src_allocation
            ) const noexcept(swp_req == expr_req::no_exception)
                requires(swp_req >= expr_req::well_formed)
            {
                get<5>(dispatchers_)(dst_alloc, dst_allocation, src_alloc, src_allocation);
            }

            [[nodiscard]] constexpr auto type() const noexcept { return current_type_; }

            [[nodiscard]] constexpr auto size() const noexcept { return type_size_; }

            [[nodiscard]] constexpr auto has_value() const noexcept { return size() != 0; }

            [[nodiscard]] constexpr operator bool() const noexcept { return has_value(); }

        private:
            dispatchers dispatchers_{};
            ::std::string_view current_type_ = type_id<void>;
            ::std::size_t type_size_{};
        };

        template<special_mem_req Req, typename Alloc>
        class object_allocation_allocator_aware : public allocator_aware_traits<Alloc>
        {
            using this_t = object_allocation_allocator_aware;

            using alloc_traits = allocator_aware_traits<Alloc>;
            using m_allocation = typename alloc_traits::allocation;
            using typename alloc_traits::allocator_type;
            using spec_mem_traits = allocation_dispatcher_traits<Req, Alloc>;

            indexed_values<allocator_type, spec_mem_traits> compressed_{};
            m_allocation allocation_;

            constexpr auto& get_allocator() noexcept { return stdsharp::get<0>(compressed_); }

        protected:
            constexpr auto& special_member_traits() noexcept
            {
                return stdsharp::get<1>(compressed_);
            }

            constexpr auto& allocation() noexcept { return allocation_; }

        public:
            object_allocation_allocator_aware() = default;

            template<special_mem_req OtherReq>
                requires constexpr
            object_allocation_allocator_aware(
                const object_allocation_allocator_aware<OtherReq, Alloc>& other
            ) noexcept:
                compressed_(other.get_allocator(), other.special_member_traits()),
                allocation_(other.allocation())
            {
            }

            constexpr auto get_allocator() const noexcept { return stdsharp::get<0>(compressed_); }
        };

        template<special_mem_req Req, allocator_req Alloc>
        class basic_object_allocation : public allocator_aware_traits<Alloc>
        {
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
                    make_allocation_by_obj<T>(get_allocator(), ptr_cast<T>(), cpp_forward(args)...)
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
            get<ValueType>() = ValueType{cpp_forward(args)...};
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

            if(type_id<value_t> == type() && try_same_type_emplace<value_t>(cpp_forward(args)...))
                return get<value_t>();

            data.assign_traits(mem_traits{::std::type_identity<T>{}});

            allocator_traits::construct(alloc, ptr_cast<value_t>(), cpp_forward(args)...);

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
            return emplace<T, decltype(il), Args...>(alloc, il, cpp_forward(args)...);
        }

        template<not_same_as<void> T>
            requires emplace_able<::std::decay_t<T>, T>
        constexpr decltype(auto) emplace(T&& t)
        {
            return emplace<::std::decay_t<T>, T>(cpp_forward(t));
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