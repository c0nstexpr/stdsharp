#pragma once

#include "allocator_aware.h"
#include "../utility/implement_dispatcher.h"
#include "../type_traits/object.h"

namespace stdsharp
{
    namespace details
    {
        template<allocation_obj_req Req, typename Alloc>
        class allocation_dispatchers
        {
            using traits = allocator_aware_traits<Alloc>;
            using alloc = traits::allocator_type;
            using alloc_cref = const alloc&;
            using allocation = traits::allocation;
            using allocation_cref = const allocation&;

            static constexpr auto cp_ctor_req = Req.copy_construct;
            static constexpr auto mov_assign_req = Req.move_assign;
            static constexpr auto cp_assign_req = Req.copy_assign;

            template<expr_req ExprReq, typename... Args>
            using ctor_dispatcher = implement_dispatcher<ExprReq, allocation, alloc&, Args...>;

            template<expr_req ExprReq, typename... Args>
            using write_dispatcher =
                implement_dispatcher<ExprReq, void, alloc&, allocation&, const bool, Args...>;

            using dispatchers = stdsharp::indexed_values<
                ctor_dispatcher<expr_req::no_exception, allocation&>,
                ctor_dispatcher<cp_ctor_req, allocation_cref>,
                write_dispatcher<mov_assign_req, alloc&, allocation&>,
                write_dispatcher<cp_assign_req, alloc_cref, allocation_cref>,
                write_dispatcher<expr_req::no_exception>,
                write_dispatcher<expr_req::well_formed> // clang-format off
            >; // clang-format on

            constexpr allocation_dispatchers(
                const dispatchers& b,
                const std::string_view current_type,
                const std::size_t type_size
            ) noexcept:
                dispatchers_(b), current_type_(current_type), type_size_(type_size)
            {
            }

        public:
            static constexpr auto req = Req;

            allocation_dispatchers() = default;

            template<
                typename T,
                typename AllocationFor = traits::template allocation_for<std::decay_t<T>>>
                requires(Req <= AllocationFor::obj_req)
            constexpr allocation_dispatchers(const std::type_identity<T>) noexcept:
                allocation_dispatchers(
                    dispatchers(
                        [](alloc& alloc, allocation& other) noexcept
                        {
                            AllocationFor allocation_for{other, true};
                            const auto res = traits::move_construct(alloc, allocation_for);
                            other = allocation_for.allocation();
                            return res.allocation();
                        },
                        [](alloc & alloc, allocation_cref other) //
                        noexcept(cp_ctor_req == expr_req::no_exception) //
                        requires(cp_ctor_req >= expr_req::well_formed) //
                        {
                            return traits::copy_construct(alloc, AllocationFor{other, true})
                                .allocation();
                        },
                        []( //
                            alloc & dst_alloc,
                            allocation & dst_allocation,
                            const bool has_value,
                            alloc& src_alloc,
                            allocation& src_allocation
                        ) noexcept(mov_assign_req == expr_req::no_exception) //
                        requires(mov_assign_req >= expr_req::well_formed) //
                        {
                            AllocationFor dst{dst_allocation, has_value};
                            AllocationFor src{src_allocation, true};

                            traits::move_assign(dst_alloc, dst, src_alloc, src);

                            dst_allocation = dst.allocation();
                            src_allocation = src.allocation();
                        },
                        []( //
                            alloc & dst_alloc,
                            allocation & dst_allocation,
                            const bool has_value,
                            alloc_cref src_alloc,
                            allocation_cref src_allocation //
                        ) noexcept(cp_assign_req == expr_req::no_exception) //
                        requires(cp_assign_req >= expr_req::well_formed) //
                        {
                            AllocationFor dst{dst_allocation, has_value};
                            traits::copy_assign(dst_alloc, dst, src_alloc, {src_allocation, true});
                            dst_allocation = dst.allocation();
                        },
                        [](alloc& alloc, allocation& allocation, const bool has_value) noexcept
                        {
                            AllocationFor dst{allocation, has_value};
                            traits::destroy(alloc, dst);
                            allocation = dst.allocation();
                        },
                        [](alloc& alloc, allocation& allocation, const bool has_value) noexcept
                        {
                            AllocationFor dst{allocation, has_value};
                            dst.shrink_to_fit(alloc);
                            allocation = dst.allocation();
                        }
                    ),
                    type_id<T>,
                    sizeof(T)
                )
            {
            }

            template<allocation_obj_req OtherReq>
            constexpr allocation_dispatchers( //
                const allocation_dispatchers<OtherReq, Alloc>& other
            ) noexcept:
                allocation_dispatchers(other.dispatchers_, other.current_type_, other.type_size_)
            {
            }

            [[nodiscard]] constexpr auto
                construct(alloc& alloc, allocation& allocation) const noexcept
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
                const bool has_value,
                alloc& src_alloc,
                allocation& src_allocation
            ) const noexcept(mov_assign_req == expr_req::no_exception)
                requires(mov_assign_req >= expr_req::well_formed)
            {
                get<2>(dispatchers_)(
                    dst_alloc,
                    dst_allocation,
                    has_value,
                    src_alloc,
                    src_allocation
                );
            }

            constexpr void assign(
                alloc& dst_alloc,
                allocation& dst_allocation,
                const bool has_value,
                alloc_cref src_alloc,
                allocation_cref src_allocation
            ) const noexcept(cp_assign_req == expr_req::no_exception)
                requires(cp_assign_req >= expr_req::well_formed)
            {
                get<3>(dispatchers_)(
                    dst_alloc,
                    dst_allocation,
                    has_value,
                    src_alloc,
                    src_allocation
                );
            }

            constexpr void
                destroy(alloc& alloc, allocation& allocation, const bool has_value) const noexcept
            {
                get<4>(dispatchers_)(alloc, allocation, has_value);
            }

            constexpr void
                shrink_to_fit(alloc& alloc, allocation& allocation, const bool has_value) const
            {
                get<5>(dispatchers_)(alloc, allocation, has_value); // NOLINT(*-magic-numbers)
            }

            [[nodiscard]] constexpr auto type() const noexcept { return current_type_; }

            [[nodiscard]] constexpr auto size() const noexcept { return type_size_; }

            [[nodiscard]] constexpr auto has_value() const noexcept { return size() != 0; }

            [[nodiscard]] constexpr operator bool() const noexcept { return has_value(); }

        private:
            dispatchers dispatchers_{};
            std::string_view current_type_ = type_id<void>;
            std::size_t type_size_{};
        };

        template<allocation_obj_req Req, typename Alloc>
        class allocation_rsc : // NOLINT(*-special-member-functions)
            public allocator_aware_traits<Alloc>
        {
            using traits = allocator_aware_traits<Alloc>;

            template<allocation_obj_req, typename>
            friend class allocation_resource;

        public:
            using typename traits::allocation;
            using typename traits::allocator_type;

        protected:
            using dispatchers_t = allocation_dispatchers<Req, Alloc>;

        private:
            using compressed_t = stdsharp::indexed_values<dispatchers_t, allocator_type>;

            compressed_t compressed_{};
            allocation allocation_{};

        public:
            allocation_rsc() = default;

            constexpr allocation_rsc(const allocator_type& alloc):
                compressed_(dispatchers_t{}, alloc)
            {
            }

            template<
                typename... Args,
                typename T,
                typename ValueType = std::decay_t<T>,
                typename Identity = std::type_identity<ValueType> // clang-format off
            > // clang-format on
                requires std::constructible_from<compressed_t, Identity, const allocator_type&> &&
                             std::invocable<make_allocation_by_obj_fn<T>, allocator_type&, Args...>
            constexpr allocation_rsc(
                const std::allocator_arg_t,
                const allocator_type& alloc,
                const std::in_place_type_t<T>,
                Args&&... args
            ):
                compressed_(Identity{}, alloc),
                allocation_(make_allocation_by_obj<T>(get_allocator(), cpp_forward(args)...))
            {
            }

            template<allocation_obj_req OtherReq>
                requires((OtherReq >= Req) && (Req.copy_construct >= expr_req::well_formed))
            constexpr allocation_rsc(
                const allocation_rsc<OtherReq, Alloc>& other,
                const allocator_type& alloc
            ):
                compressed_(other.get_dispatchers(), alloc),
                allocation_(
                    get_dispatchers() ?
                        get_dispatchers().construct(get_allocator(), other.get_allocation()) :
                        allocation{}
                )
            {
            }

            template<allocation_obj_req OtherReq>
                requires(OtherReq >= Req)
            constexpr allocation_rsc(
                allocation_rsc<OtherReq, Alloc>&& other,
                const allocator_type& alloc
            ) noexcept:
                compressed_(other.get_dispatchers(), alloc),
                allocation_(
                    get_dispatchers() ?
                        get_dispatchers().construct(get_allocator(), other.get_allocation()) :
                        allocation{}
                )
            {
            }

        private:
            constexpr auto& assign_impl(auto&& other)
            {
                if(this == &other) return *this;

                auto& other_dispatchers = other.get_dispatchers();

                if(other_dispatchers.has_value())
                {
                    destroy();
                    return *this;
                }

                auto& dispatchers = get_dispatchers();

                if(dispatchers.type() != other_dispatchers.type()) destroy();

                other_dispatchers.assign(
                    get_allocator(),
                    get_allocation(),
                    dispatchers.has_value(),
                    other.get_allocator(),
                    other.get_allocation()
                );

                dispatchers = other_dispatchers;

                return *this;
            }

        public:
            template<allocation_obj_req OtherReq>
                requires((OtherReq >= Req) && (Req.copy_assign >= expr_req::well_formed))
            constexpr allocation_rsc& operator=( //
                const allocation_rsc<OtherReq, Alloc>& other
            ) noexcept(Req.copy_assign >= expr_req::no_exception)
            {
                return assign_impl(other);
            }

            allocation_rsc& operator=(allocation_rsc&)
                requires false;

            template<allocation_obj_req OtherReq>
                requires((OtherReq >= Req) && (Req.move_assign >= expr_req::well_formed))
            constexpr allocation_rsc& operator=(allocation_rsc<OtherReq, Alloc>&& other) //
                noexcept(traits::propagate_on_move_v || traits::always_equal_v)
            {
                return assign_impl(other);
            }

            allocation_rsc& operator=(allocation_rsc&&) // NOLINT
                requires false;

            constexpr ~allocation_rsc()
            {
                destroy();
                get_allocation().deallocate(get_allocator());
            }

            template<allocation_obj_req OtherReq>
                requires(OtherReq >= Req)
            constexpr void swap(allocation_rsc<OtherReq, Alloc>& other) noexcept
            {
                auto& other_dispatchers = other.get_dispatchers();

                other_dispatchers.swap(
                    get_allocator(),
                    get_allocation(),
                    other.get_allocator(),
                    other.get_allocation()
                );

                std::swap(get_dispatchers(), other_dispatchers);
            }

            [[nodiscard]] constexpr auto& get_allocator() const noexcept
            {
                return stdsharp::get<1>(compressed_);
            }

            [[nodiscard]] constexpr operator bool() const noexcept
            {
                return get_dispatchers().has_value();
            }

            constexpr void destroy() noexcept
            {
                auto& dispatchers = get_dispatchers();

                if(!dispatchers) return;

                dispatchers.destroy(get_allocator(), get_allocation(), true);
                dispatchers = {};
            }

            constexpr void shrink_to_fit()
            {
                const auto& dispatchers = get_dispatchers();
                auto& alloc = get_allocator();
                auto& allocation = get_allocation();

                if(dispatchers.has_value()) dispatchers.shrink_to_fit(alloc, allocation, true);
                else allocation.deallocate(alloc);
            }

        protected:
            [[nodiscard]] constexpr auto& get_dispatchers() noexcept
            {
                return stdsharp::get<0>(compressed_);
            }

            [[nodiscard]] constexpr auto& get_dispatchers() const noexcept
            {
                return stdsharp::get<0>(compressed_);
            }

            [[nodiscard]] constexpr auto& get_allocator() noexcept
            {
                return stdsharp::get<1>(compressed_);
            }

            [[nodiscard]] constexpr auto& get_allocation() noexcept { return allocation_; }

            [[nodiscard]] constexpr auto& get_allocation() const noexcept { return allocation_; }
        };

        template<
            allocation_obj_req Req,
            allocator_req Alloc,
            typename Base = allocator_aware_ctor<allocation_rsc<Req, Alloc>>>
        class obj_allocation : public Base
        {
            using typename Base::dispatchers_t;

            using Base::get_allocation;
            using Base::get_dispatchers;
            using this_t = obj_allocation;

        public:
            template<typename T, typename... Args>
            static constexpr auto emplace_constructible =
                (Base::template construct_req<std::decay_t<T>, Args...> >= expr_req::well_formed) &&
                std::constructible_from<dispatchers_t, std::type_identity<std::decay_t<T>>>;

            using typename Base::allocator_type;
            using Base::Base;
            using Base::get_allocator;

            static constexpr auto req = Req;

            template<std::same_as<void> T = void>
            constexpr void emplace() noexcept
            {
                this->destroy();
            }

            template<typename T, typename... Args>
                requires emplace_constructible<T, Args...>
            constexpr decltype(auto) emplace(Args&&... args)
            {
                using value_t = std::decay_t<T>;

                this->destroy();
                get_allocation().allocate(get_allocator(), sizeof(value_t));

                this_t::construct(get_allocator(), ptr<value_t>(), cpp_forward(args)...);
                get_dispatchers() = dispatchers_t{std::type_identity<value_t>{}};

                return get<value_t>();
            }

            template<typename T, typename... Args, typename U>
            constexpr decltype(auto) emplace(const std::initializer_list<U> il, Args&&... args)
                requires emplace_constructible<T, decltype(il), Args...>
            {
                return emplace<T, decltype(il), Args...>(il, cpp_forward(args)...);
            }

            template<typename T>
                requires emplace_constructible<T, T>
            constexpr decltype(auto) emplace(T&& t)
            {
                return emplace<T, T>(cpp_forward(t));
            }

        private:
            template<typename T>
            [[nodiscard]] constexpr auto ptr() noexcept
            {
                return pointer_cast<T>(get_allocation().begin());
            }

            template<typename T>
            [[nodiscard]] constexpr auto ptr() const noexcept
            {
                return pointer_cast<T>(get_allocation().cbegin());
            }

        public:
            template<typename T>
            [[nodiscard]] constexpr decltype(auto) get() noexcept
            {
                return *ptr<T>();
            }

            template<typename T>
            [[nodiscard]] constexpr decltype(auto) get() const noexcept
            {
                return *ptr<T>();
            }

            [[nodiscard]] constexpr bool has_value() const noexcept { return get_dispatchers(); }

            [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }

            [[nodiscard]] constexpr auto type() const noexcept { return get_dispatchers().type(); }

            [[nodiscard]] constexpr auto size() const noexcept { return get_dispatchers().size(); }

            [[nodiscard]] constexpr auto reserved() const noexcept
            {
                return get_allocation().size();
            }
        };
    }

    template<typename T, allocator_req Alloc>
    using obj_allocation_for = details::obj_allocation<allocation_value_type_req<Alloc, T>, Alloc>;

    template<allocator_req Alloc>
    using trivial_obj_allocation = obj_allocation_for<trivial_object, Alloc>;

    template<allocator_req Alloc>
    using normal_obj_allocation = obj_allocation_for<normal_object, Alloc>;

    template<allocator_req Alloc>
    using unique_obj_allocation = obj_allocation_for<unique_object, Alloc>;
}