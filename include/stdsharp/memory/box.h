#pragma once

#include "box_allocation_value.h"
#include "../type_traits/object.h"

namespace stdsharp
{
    template<special_mem_req Req, allocator_req Alloc>
    class box
    {
        template<special_mem_req, allocator_req>
        friend class box;

        using allocation_traits = allocator_aware::allocation_traits<Alloc>;

    public:
        using allocator_type = allocation_traits::allocator_type;
        using allocator_traits = allocation_traits::allocator_traits;

    private:
        struct allocation_type
        {
            allocation_traits::pointer ptr;
            allocation_traits::size_type diff;

            [[nodiscard]] constexpr auto begin() const noexcept { return ptr; }

            [[nodiscard]] constexpr auto end() const noexcept { return ptr + diff; }

            [[nodiscard]] constexpr auto size() const noexcept { return diff; }
        };

        using adaptor_t = allocation_traits::template adaptor<std::array<allocation_type, 1>>;
        using box_allocation_value = allocator_aware::allocation_value<allocation_type, allocation_box_type<Req>>;

        adaptor_t adaptor_;
        box_allocation_value allocation_value_{};

        constexpr auto& get_allocation() noexcept { return adaptor_.allocations.front(); }

        constexpr auto& get_allocation() const noexcept { return adaptor_.allocations.front(); }

        constexpr auto& get_allocator() noexcept { return adaptor_.allocator_adt.get_allocator(); }

    public:
        constexpr auto& get_allocator() const noexcept
        {
            return adaptor_.allocator_adt.get_allocator();
        }

        static constexpr auto req = Req;

        box() = default;

        template<typename T>
        static constexpr auto is_type_compatible =
            std::constructible_from<box_allocation_value, std::in_place_type_t<T>>;

        box(const box&)
            requires false;
        box(box&&) noexcept
            requires false;

    private:
        template<special_mem_req OtherReq>
        static constexpr auto is_compatible = std::constructible_from<
            box_allocation_value,
            typename box<OtherReq, allocator_type>::box_allocation_value>;

        constexpr void deallocate() noexcept
        {
            allocation_traits::deallocate(adaptor_.to_src_allocations());
        }

    public:
        template<special_mem_req OtherReq>
        explicit(OtherReq != Req) constexpr box(const box<OtherReq, allocator_type>& other)
            requires requires {
                requires is_compatible<OtherReq>;
                adaptor_.on_construct(
                    other.adaptor_.allocations | std::views::all,
                    other.allocation_value_
                );
            }
            :
            adaptor_{.allocator_adt = other.adaptor_.allocator_adt.get_allocator()},
            allocation_value_(other.allocation_value_)
        {
            adaptor_.on_construct(
                other.adaptor_.allocations | std::views::all,
                other.allocation_value_
            );
        }

        template<special_mem_req OtherReq>
        explicit(OtherReq != Req) constexpr box(
            const box<OtherReq, allocator_type>& other,
            const allocator_type& alloc //
        )
            requires requires {
                requires is_compatible<OtherReq>;
                adaptor_.on_construct(
                    other.adaptor_.allocations | std::views::all,
                    other.allocation_value_
                );
            }
            : adaptor_{.allocator_adt = alloc}, allocation_value_(other.allocation_value_)
        {
            adaptor_.on_construct(
                other.adaptor_.allocations | std::views::all,
                other.allocation_value_
            );
        }

        template<special_mem_req OtherReq>
            requires is_compatible<OtherReq>
        explicit(OtherReq != Req) constexpr box(box<OtherReq, allocator_type>&& other) noexcept:
            adaptor_(cpp_move(other.adaptor_)), allocation_value_(other.allocation_value_)
        {
        }

        template<special_mem_req OtherReq>
            requires is_compatible<OtherReq>
        explicit(OtherReq != Req) constexpr box(
            box<OtherReq, allocator_type>&& other,
            const allocator_type& alloc //
        ) noexcept:
            adaptor_(alloc, cpp_move(other.adaptor_.allocations)),
            allocation_value_(other.allocation_value_)
        {
        }

        constexpr void reset() noexcept(
            noexcept(allocation_traits::destroy(adaptor_.to_src_allocations(), allocation_value_))
        )
            requires requires {
                allocation_traits::destroy(adaptor_.to_src_allocations(), allocation_value_);
            }
        {
            allocation_traits::destroy(adaptor_.to_src_allocations(), allocation_value_);
        }

        constexpr box& operator=(const box& other)
            requires requires {
                reset();
                allocation_traits::on_assign(
                    adaptor_.to_src_allocations(),
                    adaptor_.to_src_allocations(),
                    allocation_value_
                );
                adaptor_.on_construct(adaptor_.to_src_allocations(), allocation_value_);
            }
        {
            if(allocation_value_ == other.allocation_value_)
                allocation_traits::on_assign(
                    other.adaptor_.to_src_allocations(),
                    adaptor_.to_src_allocations(),
                    other.allocation_value_
                );
            else
            {
                reset();
                adaptor_.on_construct(other.adaptor_.to_src_allocations(), other.allocation_value_);
                allocation_value_ = other.allocation_value_;
            }

            return *this;
        }

        constexpr box& operator=(box&& other) noexcept(noexcept(allocation_traits::on_assign(
            adaptor_.to_src_allocations(),
            adaptor_.to_src_allocations(),
            allocation_value_
        )))
            requires requires {
                reset();
                allocation_traits::on_assign(
                    adaptor_.to_src_allocations(),
                    adaptor_.to_src_allocations(),
                    allocation_value_
                );
            }
        {
            if(allocation_value_ == other.allocation_value_)
                allocation_traits::on_assign(
                    other.adaptor_.to_src_allocations(),
                    adaptor_.to_src_allocations(),
                    other.allocation_value_
                );
            else
            {
                reset();
                deallocate();
                adaptor_.on_construct(other.adaptor_.to_src_allocations(), other.allocation_value_);
                allocation_value_ = other.allocation_value_;
            }

            return *this;
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

            allocation_value_ = box_allocation_value{std::in_place_type_t<T>{}};

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
            const std::allocator_arg_t /*unused*/,
            const allocator_type& alloc,
            const std::in_place_type_t<T> /*unused*/,
            auto&&... args
        )
            requires requires { emplace<T>(cpp_forward(args)...); }
            : adaptor_(alloc)
        {
            emplace<T>(cpp_forward(args)...);
        }

        template<typename T>
        constexpr box(const std::in_place_type_t<T> /*unused*/, auto&&... args)
            requires requires { emplace<T>(cpp_forward(args)...); }
        {
            emplace<T>(cpp_forward(args)...);
        }

        template<typename T>
        [[nodiscard]] constexpr T& get() noexcept
        {
            return allocator_aware::allocation_get<allocator_type, T>(get_allocation());
        }

        template<typename T>
        [[nodiscard]] constexpr const T& get() const noexcept
        {
            return allocator_aware::allocation_cget<allocator_type, T>(get_allocation());
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

        [[nodiscard]] constexpr auto size() const noexcept { return get_allocation().size(); }
    };

    template<typename T, allocator_req Alloc>
    using box_for = box<special_mem_req::for_type<T>(), Alloc>;

    template<allocator_req Alloc>
    using trivial_box = box_for<trivial_object, Alloc>;

    template<allocator_req Alloc>
    using normal_box = box_for<normal_object, Alloc>;

    template<allocator_req Alloc>
    using unique_box = box_for<unique_object, Alloc>;
}