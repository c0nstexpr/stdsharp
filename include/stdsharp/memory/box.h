#pragma once

#include "box_allocation_value.h"
#include "../type_traits/object.h"

namespace stdsharp
{
    template<special_mem_req Req, allocator_req Alloc>
    class box // NOLINTBEGIN(*-noexcept-*)
    {
        template<special_mem_req, allocator_req>
        friend class box;

        using allocation_traits = allocation_traits<Alloc>;

    public:
        using allocator_type = allocation_traits::allocator_type;

    private:
        using allocator_traits = allocation_traits::allocator_traits;

        struct allocation_type
        {
            allocation_traits::pointer ptr;
            allocation_traits::size_type diff;

            allocation_type() = default;

            constexpr allocation_type(
                const allocation_traits::pointer ptr,
                const allocation_traits::size_type diff
            ) noexcept:
                ptr(ptr), diff(diff)
            {
            }

            constexpr allocation_type(
                const allocation<allocator_type> auto other //
            ) noexcept:
                ptr(std::ranges::begin(other)), diff(std::ranges::size(other))
            {
            }

            [[nodiscard]] constexpr auto begin() const noexcept { return ptr; }

            [[nodiscard]] constexpr auto end() const noexcept { return ptr + diff; }

            [[nodiscard]] constexpr auto size() const noexcept { return diff; }
        };

        using allocation_value =
            allocation_value<allocation_type, allocation_box_type<Req>>;

        using allocations_type = std::array<allocation_type, 1>;

        using allocations_view_t = std::ranges::ref_view<allocations_type>;

        using callocations_view_t = std::invoke_result_t<
            make_callocations_fn<allocator_type>,
            std::ranges::ref_view<const allocations_type>>;

        allocator_traits::adaptor allocator_adaptor_{};
        allocations_type allocations_;
        allocation_value allocation_value_{};

        constexpr allocations_view_t get_allocations() noexcept { return allocations_; }

        constexpr callocations_view_t get_allocations() const noexcept
        {
            return  //
                make_callocations<allocator_type>(std::ranges::ref_view{allocations_});
        }

        constexpr auto& get_allocation() noexcept { return get_allocations().front(); }

        constexpr auto get_allocation() const noexcept { return get_allocations().front(); }

        constexpr auto& get_allocator() noexcept { return allocator_adaptor_.get_allocator(); }

        constexpr allocation_traits::template src_allocations<allocations_view_t>
            to_src_allocations() noexcept
        {
            return {get_allocator(), get_allocations()};
        }

        constexpr allocation_traits::template src_callocations<callocations_view_t>
            to_src_allocations() const noexcept
        {
            return {get_allocator(), get_allocations()};
        }

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
            allocation_traits::deallocate(to_src_allocations());
        }

        template<special_mem_req OtherReq>
        constexpr void on_copy_construct(const box<OtherReq, allocator_type>& other)
            requires requires {
                requires is_compatible<OtherReq>;
                allocation_traits::on_construct(
                    get_allocations(),
                    to_src_allocations(),
                    allocation_value_
                );
            }
        {
            allocation_traits::on_construct(
                other.get_allocations(),
                to_src_allocations(),
                other.allocation_value_
            );

            allocation_value_ = other.allocation_value_;
        }

    public:
        template<special_mem_req OtherReq>
        explicit(OtherReq != Req) constexpr box(const box<OtherReq, allocator_type>& other)
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
        explicit(OtherReq != Req) constexpr box(
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

        constexpr void reset() //
            noexcept(noexcept(allocation_traits::destroy(to_src_allocations(), allocation_value_)))
            requires requires {
                allocation_traits::destroy(to_src_allocations(), allocation_value_);
            }
        {
            allocation_traits::destroy(to_src_allocations(), allocation_value_);
            allocation_value_ = {};
        }

    private:
        constexpr void allocate(const std::size_t size)
        {
            allocations_[0] =
                allocation_traits::template allocate<allocation_type>(get_allocator(), size);
        }

        constexpr void prepare_allocation(const box& other)
            requires allocator_traits::propagate_on_copy_v
        {
            if constexpr(!allocator_traits::always_equal_v)
                if(get_allocator() != other.get_allocator())
                {
                    deallocate();
                    get_allocator() = other.get_allocator();
                    allocate(other.size());

                    return;
                }

            if(capacity() < other.size())
            {
                deallocate();
                get_allocator() = other.get_allocator();
                allocate(other.size());
            }
        }

        constexpr void prepare_allocation(const box& other)
        {
            if(capacity() < other.size())
            {
                deallocate();
                allocate(other.size());
            }
        }

    public:
        constexpr box& operator=(const box& other)
            requires requires {
                reset();
                allocation_traits::on_assign(
                    to_src_allocations(),
                    to_src_allocations(),
                    allocation_value_
                );
                allocation_traits::on_construct(
                    other.get_allocations(),
                    to_src_allocations(),
                    allocation_value_
                );
            }
        {
            if(allocation_value_ == other.allocation_value_)
                allocation_traits::on_assign(
                    other.to_src_allocations(),
                    to_src_allocations(),
                    allocation_value_
                );
            else
            {
                reset();
                prepare_allocation(other);

                allocation_traits::on_construct(
                    other.get_allocations(),
                    to_src_allocations(),
                    other.allocation_value_
                );
                allocation_value_ = other.allocation_value_;
            }

            return *this;
        }

    private:
        static constexpr auto mov_allocation_v =
            allocator_traits::propagate_on_move_v || allocator_traits::always_equal_v;

        constexpr void on_different_type_mov_assign(box&& other) noexcept
            requires mov_allocation_v
        {
            deallocate();

            allocations_ = other.allocations_;

            if constexpr(allocator_traits::propagate_on_move_v)
                get_allocator() = cpp_move(other.get_allocator());
        }

        constexpr void on_different_type_mov_assign(box& other)
            requires requires {
                requires !mov_allocation_v;
                allocation_traits::on_construct(
                    get_allocations(),
                    to_src_allocations(),
                    allocation_value_
                );
            }
        {
            if(capacity() < other.size())
            {
                deallocate();
                allocate(other.size());
            }

            allocation_traits::on_construct(
                other.get_allocations(),
                to_src_allocations(),
                other.allocation_value_
            );
        }

    public:
        constexpr box& operator=(box&& other) noexcept( //
            noexcept( //
                allocation_traits::on_assign(
                    to_src_allocations(),
                    to_src_allocations(),
                    allocation_value_
                ),
                reset(),
                on_different_type_mov_assign(other)
            )
        )
            requires requires {
                reset();
                allocation_traits::on_assign(
                    to_src_allocations(),
                    to_src_allocations(),
                    allocation_value_
                );
                on_different_type_mov_assign(other);
            }
        {
            if(allocation_value_ == other.allocation_value_)
                allocation_traits::on_assign(
                    other.to_src_allocations(),
                    to_src_allocations(),
                    other.allocation_value_
                );
            else
            {
                reset();
                on_different_type_mov_assign(other);
                allocation_value_ = other.allocation_value_;
            }

            return *this;
        }

        constexpr ~box() noexcept(noexcept(reset()))
            requires requires { reset(); }
        {
            reset();
            deallocate();
        }

        friend constexpr void swap(box& lhs, box& rhs) noexcept
        {
            allocation_traits::on_swap(lhs.to_src_allocations(), rhs.to_src_allocations());
            std::swap(lhs.allocation_value_, rhs.allocation_value_);
        }

        template<typename T, typename... U>
            requires is_type_compatible<T>
        constexpr decltype(auto) emplace(U&&... args)
            requires requires {
                allocation_traits::template construct<T>(
                    adaptor_.to_src_allocations(),
                    cpp_forward(args)...
                );
            }
        {
            allocation_traits::template construct<T>(
                adaptor_.to_src_allocations(),
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