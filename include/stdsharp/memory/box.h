#pragma once

#include "allocation_value.h"
#include "../type_traits/object.h"
#include "stdsharp/type_traits/core_traits.h"

namespace stdsharp
{
    template<special_mem_req Req, allocator_req Alloc>
    class box // NOLINTBEGIN(*-noexcept-*)
    {
        template<special_mem_req, allocator_req>
        friend class box;

        friend constexpr void swap(box& lhs, box& rhs) noexcept
        {
            lhs.alloc_adaptor_.swap_with(rhs.get_allocator());
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

        allocator_traits::adaptor alloc_adaptor_{};
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

        constexpr auto& get_allocation() const noexcept { return get_allocations().front(); }

        constexpr auto& get_allocator() noexcept { return alloc_adaptor_.get_allocator(); }

    public:
        constexpr auto& get_allocator() const noexcept { return alloc_adaptor_.get_allocator(); }

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
            typename box<OtherReq, allocator_type>::allocation_value>;

        constexpr void deallocate() noexcept
        {
            if(allocation_traits::empty(get_allocation())) return;
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
            alloc_adaptor_(other.get_allocator()),
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
            alloc_adaptor_(std::in_place, alloc),
            allocations_(
                allocation_traits::template allocate<allocation_type>(get_allocator(), other.size())
            )
        {
            on_copy_construct(other);
        }

        template<special_mem_req OtherReq>
            requires is_compatible<OtherReq>
        explicit(OtherReq != Req) constexpr box(box<OtherReq, allocator_type>&& other) noexcept:
            alloc_adaptor_(cpp_move(other.get_allocator())),
            allocation_value_(other.allocation_value_)
        {
        }

        template<special_mem_req OtherReq>
            requires is_compatible<OtherReq>
        explicit(OtherReq != Req) constexpr box(
            box<OtherReq, allocator_type>&& other,
            const allocator_type& alloc //
        ) noexcept:
            alloc_adaptor_(std::in_place, alloc), allocation_value_(other.allocation_value_)
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
            requires requires {
                allocation_traits::on_destroy(
                    get_allocator(),
                    get_allocations_view(),
                    allocation_value_
                );
            }
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

        static constexpr void copy_construct(box& instance, const box& other)
            requires requires {
                allocation_traits::on_construct(
                    instance.get_allocator(),
                    instance.get_allocations_view(),
                    other.get_allocations_view(),
                    other.allocation_value_
                );
            }
        {
            allocation_traits::on_construct(
                instance.get_allocator(),
                instance.get_allocations_view(),
                other.get_allocations_view(),
                other.allocation_value_
            );
        }

        static constexpr void copy_assign(box& instance, const box& other)
            requires requires {
                allocation_traits::on_assign(
                    instance.get_allocations_view(),
                    other.get_allocations_view(),
                    other.allocation_value_
                );
            }
        {
            allocation_traits::on_assign(
                instance.get_allocations_view(),
                other.get_allocations_view(),
                other.allocation_value_
            );
        }

        static constexpr void cp_assign(
            box& instance,
            const box& other,
            const allocator_propagation<> /*unused*/
        )
            requires requires {
                copy_assign(instance, other);
                instance.reset();
                copy_construct(instance, other);
            }
        {
            if(instance.allocation_value_ == other.allocation_value_)
            {
                copy_assign(instance, other);
                return;
            }

            instance.reset();

            if(const auto size = other.size(); instance.capacity() < size)
            {
                instance.deallocate();
                instance.allocate(size);
            }

            copy_construct(instance, other);
        }

        static constexpr void cp_assign(
            box& instance,
            const box& other,
            const allocator_propagation<true> propagation
        )
            requires requires { cp_assign(instance, other, allocator_propagation<>{}); }
        {
            if(propagation.assigned) cp_assign(instance, other, allocator_propagation<>{});
        }

        static constexpr void cp_assign(
            box& instance,
            const box& other,
            const allocator_propagation<false> propagation
        )
            requires requires {
                copy_construct(instance, other);
                instance.reset();
            }
        {
            if(propagation.assigned)
            {
                instance.allocate(other.size());
                copy_construct(instance, other);
            }
            else
            {
                instance.reset();
                instance.deallocate();
            }
        }

        struct cp_assign_fn
        {
            std::reference_wrapper<box> instance;
            std::reference_wrapper<const box> other;

            constexpr void operator()(const auto propagation, allocator_type& /*unused*/) const //
                noexcept(noexcept(cp_assign(instance, other, propagation)))
                requires requires { cp_assign(instance, other, propagation); }
            {
                cp_assign(instance, other, propagation);
            }
        };

        static constexpr void move_allocation(box& instance, box& other)
        {
            instance.get_allocation() =
                std::exchange(other.get_allocation(), empty_allocation_result<allocator_type>);
        }

        static constexpr void move_construct(box& instance, box& other)
            requires requires {
                allocation_traits::on_construct(
                    instance.get_allocator(),
                    instance.get_allocations_view(),
                    other.get_allocations_view(),
                    other.allocation_value_
                );
            }
        {
            allocation_traits::on_construct(
                instance.get_allocator(),
                instance.get_allocations_view(),
                other.get_allocations_view(),
                other.allocation_value_
            );
        }

        static constexpr void move_assign(box& instance, box& other)
            requires requires {
                allocation_traits::on_assign(
                    instance.get_allocations_view(),
                    other.get_allocations_view(),
                    other.allocation_value_
                );
            }
        {
            allocation_traits::on_assign(
                instance.get_allocations_view(),
                other.get_allocations_view(),
                other.allocation_value_
            );
        }

        template<bool IsEqual>
        static constexpr void mov_assign(
            box& instance,
            box& other,
            const allocator_propagation<IsEqual> propagation,
            allocator_type& /*unused*/
        ) noexcept(noexcept(instance.reset()))
            requires requires { instance.reset(); }
        {
            if(propagation.assigned)
            {
                move_allocation(instance, other);
                return;
            }

            instance.reset();
            instance.deallocate();
        }

        static constexpr void mov_assign(
            box& instance,
            box& other,
            const allocator_propagation<> /*unused*/
        ) noexcept(noexcept(instance.reset()))
            requires requires {
                requires allocator_traits::always_equal_v;
                instance.reset();
            }
        {
            instance.reset();
            instance.deallocate();
            move_allocation(instance, other);
        }

        static constexpr void mov_assign(
            box& instance,
            box& other,
            const allocator_propagation<> /*unused*/
        ) noexcept(noexcept(instance.reset()))
            requires requires {
                instance.reset();
                move_construct(instance, other);
            }
        {
            if(instance.get_allocator() == other.get_allocator())
            {
                instance.reset();
                instance.deallocate();
                move_allocation(instance, other);
                return;
            }

            if(instance.allocation_value_ == other.allocation_value_)
            {
                move_assign(instance, other);
                return;
            }

            instance.reset();

            if(const auto size = other.size(); instance.capacity() < size)
            {
                instance.deallocate();
                instance.allocate(size);
            }

            move_construct(instance, other);
        }

        struct mov_assign_fn
        {
            std::reference_wrapper<box> instance;
            std::reference_wrapper<box> other;

            constexpr void operator()(const auto propagation, allocator_type& /*unused*/) const //
                noexcept(noexcept(mov_assign(instance, other, propagation)))
                requires requires { mov_assign(instance, other, propagation); }
            {
                mov_assign(instance, other, propagation);
            }
        };

    public:
        box& operator=(const box&)
            requires false;
        box& operator=(box&&)
            requires false;

        constexpr box& operator=(const std::same_as<box> auto& other)
            requires requires {
                alloc_adaptor_.assign(other.get_allocator(), cp_assign_fn{*this, other});
            }
        {
            alloc_adaptor_.assign(other.get_allocator(), cp_assign_fn{*this, other});
            allocation_value_ = other.allocation_value_;
            return *this;
        }

        constexpr box& operator=(std::same_as<box> auto&& other) noexcept(
            noexcept(alloc_adaptor_.assign(other.get_allocator(), mov_assign_fn{*this, other}))
        )
            requires requires {
                alloc_adaptor_.assign(other.get_allocator(), mov_assign_fn{*this, other});
            }
        {
            alloc_adaptor_.assign(cpp_move(other.get_allocator()), mov_assign_fn{*this, other});
            allocation_value_ = other.allocation_value_;
            return *this;
        }

        constexpr ~box() noexcept(noexcept(reset()))
            requires requires { reset(); }
        {
            reset();
            deallocate();
        }

        template<typename T, typename... U>
        constexpr decltype(auto) emplace(U&&... args)
            requires requires {
                requires is_type_compatible<T>;
                reset();
                allocation_traits::template construct<T>(
                    get_allocator(),
                    get_allocation(),
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
                get_allocation(),
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
            : alloc_adaptor_(std::in_place, alloc)
        {
            emplace<T>(cpp_forward(args)...);
        }

        template<typename T>
        constexpr box(const std::in_place_type_t<T> tag, auto&&... args)
            requires requires { box({}, tag, cpp_forward(args)...); }
            : box({}, tag, cpp_forward(args)...)
        {
        }

        constexpr box(const allocator_type& alloc) noexcept: alloc_adaptor_(std::in_place, alloc) {}

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
            if constexpr(is_type_compatible<T>)
                return allocation_value_ == allocation_value{std::in_place_type_t<T>{}};
            else return false;
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