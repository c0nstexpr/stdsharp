#pragma once

#include "allocation_value.h"
#include "../type_traits/object.h"

namespace stdsharp
{
    template<special_mem_req Req, allocator_req Alloc>
    class box // NOLINTBEGIN(*-noexcept-*)
    {
        template<special_mem_req, allocator_req>
        friend class box;

        friend constexpr void swap(box& lhs, box& rhs) noexcept
        {
            lhs.allocator_adaptor_.swap_with(rhs.get_allocator());
            std::swap(lhs.get_allocation(), rhs.get_allocation());
            std::swap(lhs.allocation_value_, rhs.allocation_value_);
        }

        using allocation_traits = allocation_traits<Alloc>;

    public:
        using allocator_type = allocation_traits::allocator_type;

    private:
        using allocator_traits = allocation_traits::allocator_traits;

        using allocation_type = allocation_traits::allocation_result;

        using callocation_type = allocation_traits::callocation_result;

        using allocation_value = allocation_value<
            allocator_type,
            allocation_dynamic_type<Req, allocation_type, callocation_type>>;

        using allocations_type = std::array<allocation_type, 1>;

        allocator_traits::adaptor allocator_adaptor_{};
        allocations_type allocations_;
        allocation_value allocation_value_{};

        constexpr auto& get_allocations() noexcept { return allocations_; }

        constexpr auto& get_allocations() const noexcept { return allocations_; }

        constexpr auto& get_allocations_view() noexcept { return get_allocations(); }

        constexpr auto get_allocations_view() const noexcept
        {
            return get_allocations() | views::cast<callocation_type>;
        }

        constexpr auto& get_allocation() noexcept { return get_allocations().front(); }

        constexpr auto get_allocation() const noexcept
        {
            return make_callocation<allocator_type>(get_allocations().front());
        }

        constexpr auto& get_allocator() noexcept { return allocator_adaptor_.get_allocator(); }

    public:
        constexpr auto& get_allocator() const noexcept
        {
            return allocator_adaptor_.get_allocator();
        }

        static constexpr auto req = Req;

        box() = default;

        template<typename T>
        static constexpr auto is_type_compatible =
            std::constructible_from<allocation_value, std::in_place_type_t<T>>;

        box(const box&)
            requires false;
        box(box&&) noexcept
            requires false;

    private:
        template<special_mem_req OtherReq>
        static constexpr auto is_compatible = std::constructible_from<
            allocation_value,
            typename box<OtherReq, allocator_type>::box_allocation_value>;

        constexpr void deallocate() noexcept
        {
            allocation_traits::deallocate(get_allocator(), get_allocations_view());
        }

        template<special_mem_req OtherReq>
        constexpr void on_copy_construct(const box<OtherReq, allocator_type>& other)
            requires requires {
                requires is_compatible<OtherReq>;
                allocation_traits::on_construct(
                    get_allocator(),
                    get_allocations_view(),
                    get_allocations_view(),
                    allocation_value_
                );
            }
        {
            allocation_traits::on_construct(
                get_allocator(),
                other.get_allocations_view(),
                get_allocations_view(),
                other.allocation_value_
            );

            allocation_value_ = other.allocation_value_;
        }

    public:
        template<special_mem_req OtherReq>
        explicit(OtherReq != req) constexpr box(const box<OtherReq, allocator_type>& other)
            requires requires { on_copy_construct(other); }
            :
            allocator_adaptor_(other.get_allocator()),
            allocations_(
                allocation_traits::template allocate<allocation_type>(get_allocator(), other.size())
            )
        {
            on_copy_construct(other);
        }

        template<special_mem_req OtherReq>
        explicit(OtherReq != req) constexpr box(
            const box<OtherReq, allocator_type>& other,
            const allocator_type& alloc //
        )
            requires requires { on_copy_construct(other); }
            :
            allocator_adaptor_(std::in_place, alloc),
            allocations_(
                allocation_traits::template allocate<allocation_type>(get_allocator(), other.size())
            )
        {
            on_copy_construct(other);
        }

        template<special_mem_req OtherReq>
            requires is_compatible<OtherReq>
        explicit(OtherReq != Req) constexpr box(box<OtherReq, allocator_type>&& other) noexcept:
            allocator_adaptor_(cpp_move(other.get_allocator())),
            allocation_value_(other.allocation_value_)
        {
        }

        template<special_mem_req OtherReq>
            requires is_compatible<OtherReq>
        explicit(OtherReq != Req) constexpr box(
            box<OtherReq, allocator_type>&& other,
            const allocator_type& alloc //
        ) noexcept:
            allocator_adaptor_(std::in_place, alloc), allocation_value_(other.allocation_value_)
        {
        }

        constexpr void reset() noexcept( //
            noexcept( //
                allocation_traits::on_destroy(
                    get_allocator(),
                    get_allocations_view(),
                    allocation_value_
                )
            )
        )
        // requires requires {
        //     allocation_traits::on_destroy(
        //         get_allocator(),
        //         get_allocations_view(),
        //         allocation_value_
        //     );
        // }
        {
            allocation_traits::on_destroy(
                get_allocator(),
                get_allocations_view(),
                allocation_value_
            );
            allocation_value_ = {};
        }

    private:
        constexpr void allocate(const std::size_t size)
        {
            get_allocation() =
                allocation_traits::template allocate<allocation_type>(get_allocator(), size);
        }

        struct cp_assign_fn
        {
            std::reference_wrapper<box> instance;
            std::reference_wrapper<const box> other;

            constexpr box& get_instance() const noexcept { return instance.get(); }

            constexpr box& get_other() const noexcept { return other.get(); }

            constexpr void copy_assign() const
                requires requires {
                    allocation_traits::on_assign(
                        get_instance().get_allocations_view(),
                        get_other().get_allocations_view(),
                        get_other().allocation_value_
                    );
                }
            {
                allocation_traits::on_assign(
                    get_instance().get_allocations_view(),
                    get_other().get_allocations_view(),
                    get_other().allocation_value_
                );
            }

            constexpr void copy_construct(allocator_type& alloc) const
                requires requires {
                    allocation_traits::on_construct(
                        alloc,
                        get_instance().get_allocations_view(),
                        get_other().get_allocations_view(),
                        get_other().allocation_value_
                    );
                }
            {
                allocation_traits::on_construct(
                    alloc,
                    get_instance().get_allocations_view(),
                    get_other().get_allocations_view(),
                    get_other().allocation_value_
                );
            }

            constexpr void
                operator()(const allocator_propagation<> /*unused*/, allocator_type& alloc) const
                requires requires {
                    copy_assign();
                    get_instance().reset();
                    copy_construct(alloc);
                }
            {
                if(get_instance().allocation_value_ == get_other().allocation_value_)
                {
                    copy_assign();
                    return;
                }

                get_instance().reset();

                if(const auto size = get_other().size(); get_instance().capacity() < size)
                {
                    get_instance().deallocate();
                    get_instance().get_allocation() =
                        allocation_traits::template allocate<allocation_type>(alloc, size);
                }

                copy_construct(alloc);
            }

            constexpr void operator()(
                const allocator_propagation<true> propagation,
                allocator_type& alloc //
            ) const
                requires std::invocable<cp_assign_fn, allocator_propagation<>, allocator_type&>
            {
                if(propagation.assigned) (*this)(allocator_propagation<>{}, alloc);
            }

            constexpr void operator()(
                const allocator_propagation<false> propagation,
                allocator_type& alloc //
            ) const
                requires requires {
                    get_instance().reset();
                    copy_construct(alloc);
                }
            {
                if(propagation.assigned)
                {
                    get_instance().get_allocation() =
                        allocation_traits::template allocate<allocation_type>(
                            alloc,
                            get_other().size()
                        );
                    copy_construct(alloc);
                }
                else
                {
                    get_instance().reset();
                    get_instance().deallocate();
                }
            }
        };

        struct mov_assign_fn
        {
            std::reference_wrapper<box> instance;
            std::reference_wrapper<box> other;

            constexpr box& get_instance() const noexcept { return instance.get(); }

            constexpr box& get_other() const noexcept { return other.get(); }

            constexpr void deallocate() const noexcept(noexcept(get_instance().reset()))
                requires requires { get_instance().reset(); }
            {
                get_instance().reset();
                get_instance().deallocate();
            }

            constexpr void move_allocation() const noexcept
            {
                get_instance().get_allocation() = std::exchange(get_other().get_allocation(), {});
            }

            constexpr void move_construct(allocator_type& alloc) const
                requires requires {
                    allocation_traits::on_construct(
                        alloc,
                        get_instance().get_allocations_view(),
                        get_other().get_allocations_view(),
                        get_other().allocation_value_
                    );
                }
            {
                allocation_traits::on_construct(
                    alloc,
                    get_instance().get_allocations_view(),
                    get_other().get_allocations_view(),
                    get_other().allocation_value_
                );
            }

            constexpr void move_assign() const
                requires requires {
                    allocation_traits::on_assign(
                        get_instance().get_allocations_view(),
                        get_other().get_allocations_view(),
                        get_other().allocation_value_
                    );
                }
            {
                allocation_traits::on_assign(
                    get_instance().get_allocations_view(),
                    get_other().get_allocations_view(),
                    get_other().allocation_value_
                );
            }

            template<bool IsEqual>
            constexpr void operator()(
                const allocator_propagation<IsEqual> propagation,
                allocator_type& /*unused*/
            ) const noexcept(noexcept(deallocate()))
                requires requires { deallocate(); }
            {
                if(propagation.assigned)
                {
                    move_allocation();
                    return;
                }

                deallocate();
            }

            constexpr void operator()(
                const allocator_propagation<> /*unused*/,
                allocator_type& /*unused*/
            ) const noexcept(noexcept(deallocate()))
                requires requires {
                    requires allocator_traits::always_equal_v;
                    deallocate();
                }
            {
                move_allocation();
                deallocate();
            }

            constexpr void
                operator()(const allocator_propagation<> /*unused*/, allocator_type& alloc) const
                requires requires {
                    deallocate();
                    move_construct(alloc);
                }
            {
                if(alloc == get_other().get_allocator())
                {
                    move_allocation();
                    deallocate();
                    return;
                }

                if(get_instance().allocation_value_ == get_other().allocation_value_)
                {
                    move_assign();
                    return;
                }

                get_instance().reset();

                if(const auto size = get_other().size(); get_instance().capacity() < size)
                {
                    get_instance().deallocate();
                    get_instance().get_allocation() =
                        allocation_traits::template allocate<allocation_type>(alloc, size);
                }

                move_construct(alloc);
            }
        };

    public:
        constexpr box& operator=(const box& other)
            requires requires {
                allocator_adaptor_.assign(other.get_allocator(), cp_assign_fn{*this, other});
            }
        {
            allocator_adaptor_.assign(other.get_allocator(), cp_assign_fn{*this, other});
            allocation_value_ = other.allocation_value_;
            return *this;
        }

        constexpr box& operator=(box&& other)
            requires requires {
                allocator_adaptor_.assign(other.get_allocator(), mov_assign_fn{*this, other});
            }
        {
            allocator_adaptor_.assign(cpp_move(other.get_allocator()), mov_assign_fn{*this, other});
            allocation_value_ = other.allocation_value_;
            return *this;
        }

        constexpr ~box() noexcept(noexcept(reset()))
            requires requires { this->reset(); }
        {
            reset();
            deallocate();
        }

        template<typename T, typename... U>
            requires is_type_compatible<T>
        constexpr decltype(auto) emplace(U&&... args)
            requires requires {
                reset();
                allocation_traits::template construct<T>(
                    get_allocator(),
                    get_allocations_view(),
                    cpp_forward(args)...
                );
            }
        {
            reset();

            if(const auto size = sizeof(T); capacity() < size)
            {
                deallocate();
                allocate(size);
            }

            allocation_traits::template construct<T>(
                get_allocator(),
                get_allocations_view(),
                cpp_forward(args)...
            );

            allocation_value_ = allocation_value{std::in_place_type_t<T>{}};
            return get<T>();
        }

        template<
            typename T,
            typename U,
            typename... Args,
            typename IL = const std::initializer_list<U>&>
        constexpr decltype(auto) emplace(const std::initializer_list<U> il, Args&&... args) //
            noexcept(noexcept(emplace<T, IL, Args...>(il, cpp_forward(args)...)))
            requires requires { emplace<T, IL, Args...>(il, cpp_forward(args)...); }
        {
            return emplace<T, IL, Args...>(il, cpp_forward(args)...);
        }

        template<typename T>
        constexpr decltype(auto) emplace(T&& t) //
            noexcept(noexcept(emplace<std::decay_t<T>, T>(cpp_forward(t))))
            requires requires { emplace<std::decay_t<T>, T>(cpp_forward(t)); }
        {
            return emplace<std::decay_t<T>, T>(cpp_forward(t));
        }

        template<typename T>
        constexpr box(
            const allocator_type& alloc,
            const std::in_place_type_t<T> /*unused*/,
            auto&&... args
        )
            requires requires { emplace<T>(cpp_forward(args)...); }
            : allocator_adaptor_(std::in_place, alloc)
        {
            emplace<T>(cpp_forward(args)...);
        }

        template<typename T>
        constexpr box(const std::in_place_type_t<T> tag, auto&&... args)
            requires requires { box({}, tag, cpp_forward(args)...); }
            : box({}, tag, cpp_forward(args)...)
        {
        }

        template<typename T>
        [[nodiscard]] constexpr T& get() noexcept
        {
            return allocation_traits::template get<T>(get_allocation());
        }

        template<typename T>
        [[nodiscard]] constexpr const T& get() const noexcept
        {
            return allocation_traits::template cget<T>(get_allocation());
        }

        [[nodiscard]] constexpr bool has_value() const noexcept
        {
            return !allocation_value_.empty();
        }

        template<typename T>
        [[nodiscard]] constexpr auto is_type() const noexcept
        {
            return allocation_value_.template is_type<T>();
        }

        [[nodiscard]] constexpr auto size() const noexcept
        {
            return allocation_value_.value_size();
        }

        [[nodiscard]] constexpr auto capacity() const noexcept { return get_allocation().size(); }
    }; // NOLINTEND(*-noexcept-*)

    template<typename T, typename Alloc>
    box(const Alloc&, std::in_place_type_t<T>, auto&&...)
        -> box<special_mem_req::for_type<T>(), Alloc>;

    template<typename T, allocator_req Alloc>
    using box_for = box<special_mem_req::for_type<T>(), Alloc>;

    template<allocator_req Alloc>
    struct make_box_fn
    {
        template<typename T>
            requires std::constructible_from<box_for<std::decay_t<T>, Alloc>, T>
        constexpr box_for<std::decay_t<T>, Alloc> operator()(T&& t) const
        {
            using decay_t = std::decay_t<T>;
            return {std::in_place_type<decay_t>, cpp_forward(t)};
        }
    };

    template<allocator_req Alloc>
    inline constexpr make_box_fn<Alloc> make_box{};

    template<allocator_req Alloc>
    using trivial_box = box_for<trivial_object, Alloc>;

    template<allocator_req Alloc>
    using normal_box = box_for<normal_object, Alloc>;

    template<allocator_req Alloc>
    using unique_box = box_for<unique_object, Alloc>;
}