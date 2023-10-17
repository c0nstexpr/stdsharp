#pragma once

#include "details/box_allocation.h"
#include "allocator_aware/basic_aa.h"
#include "../type_traits/object.h"
#include "stdsharp/concepts/concepts.h"

namespace stdsharp
{
    template<special_mem_req Req, allocator_req Alloc>
    class box : allocator_aware::basic_aa<details::box_allocation<Req, Alloc>>
    {
        template<special_mem_req, allocator_req>
        friend class box;

        using m_base = allocator_aware::basic_aa<details::box_allocation<Req, Alloc>>;
        using typename m_base::allocation_traits;
        using typename m_base::allocation_type;
        using typename m_base::callocation;
        using typename m_base::allocator_traits;

        using m_base::get_allocation;
        using m_base::get_allocator;

    public:
        using typename m_base::allocator_type;

        static constexpr auto req = details::box_allocation<Req, Alloc>::req;

        using m_base::m_base;

        using m_base::destroy;

        box() = default;

        template<typename T>
        constexpr box(
            const std::allocator_arg_t /*unused*/,
            const allocator_type& alloc,
            const std::in_place_type_t<T> /*unused*/,
            auto&&... args
        ) noexcept(noexcept(emplace<T>(cpp_forward(args)...)))
            requires requires { emplace<T>(cpp_forward(args)...); }
            : m_base(alloc)
        {
            emplace<T>(cpp_forward(args)...);
        }

        template<typename T>
        constexpr box(const std::in_place_type_t<T> /*unused*/, auto&&... args) //
            noexcept(noexcept(emplace<T>(cpp_forward(args)...)))
            requires requires { emplace<T>(cpp_forward(args)...); }
        {
            emplace<T>(cpp_forward(args)...);
        }

        template<special_mem_req OtherReq, typename OtherBox = box<OtherReq, allocator_type>>
        explicit constexpr box(const box<OtherReq, allocator_type>& other) //
            noexcept(noexcept(other.get_allocation().template cp_construct<Req>(get_allocator())))
            requires requires(allocator_type alloc) {
                requires OtherReq != Req;
                other.get_allocation().template cp_construct<Req>(alloc);
            }
            :
            m_base(
                cpp_move(other.get_allocator()),
                [&allocation = other.get_allocation()](auto& alloc)
                {
                    return allocation.template cp_construct<Req>(alloc); //
                }
            )
        {
        }

        template<special_mem_req OtherReq>
        explicit constexpr box(
            const box<OtherReq, allocator_type>& other,
            const allocator_type& alloc
        ) noexcept(noexcept(other.get_allocation().template cp_construct<Req>(alloc)))
            requires requires {
                requires OtherReq != Req;
                other.get_allocation().template cp_construct<Req>(alloc);
            }
            :
            m_base(
                alloc,
                [&allocation = other.get_allocation()](auto& alloc)
                {
                    return allocation.template cp_construct<Req>(alloc); //
                }
            )
        {
        }

        template<special_mem_req OtherReq, typename OtherBox = box<OtherReq, allocator_type>>
        explicit constexpr box(box<OtherReq, allocator_type>&& other) //
            noexcept(noexcept(other.get_allocation().template mov_construct<Req>(get_allocator())))
            requires requires(allocator_type alloc) {
                requires OtherReq != Req;
                other.get_allocation().template mov_construct<Req>(alloc);
            }
            :
            m_base(
                cpp_move(other.get_allocator()),
                [&allocation = other.get_allocation()](auto& alloc)
                {
                    return allocation.template mov_construct<Req>(alloc); //
                }
            )
        {
        }

        template<special_mem_req OtherReq>
        explicit constexpr box(
            box<OtherReq, allocator_type>&& other,
            const allocator_type& alloc
        ) noexcept(noexcept(other.get_allocation().template mov_construct<Req>(alloc)))
            requires requires {
                requires OtherReq != Req;
                other.get_allocation().template mov_construct<Req>(alloc);
            }
            :
            m_base(
                alloc,
                [&allocation = other.get_allocation()](auto& alloc)
                {
                    return allocation.template mov_construct<Req>(alloc); //
                }
            )
        {
        }

        [[nodiscard]] constexpr operator bool() const noexcept { return has_value(); }

        template<typename T, typename... U>
        constexpr decltype(auto) emplace(U&&... args) //
            noexcept(noexcept(this->construct<T>(cpp_forward(args)...)))
            requires requires { this->construct<T>(cpp_forward(args)...); }
        {
            return this->construct<T>(cpp_forward(args)...);
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
        [[nodiscard]] constexpr T& get() noexcept
        {
            return get_allocation().template get<T>();
        }

        template<typename T>
        [[nodiscard]] constexpr const T& get() const noexcept
        {
            return get_allocation().template get<const T>();
        }

        [[nodiscard]] constexpr bool has_value() const noexcept
        {
            return !allocation_traits::empty(get_allocation());
        }

        template<typename T>
        [[nodiscard]] constexpr auto is_type() const noexcept
        {
            return get_allocation().template is_type<T>();
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