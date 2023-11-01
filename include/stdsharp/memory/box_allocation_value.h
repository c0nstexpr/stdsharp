#pragma once

#include "allocator_aware/allocation_value.h"

namespace stdsharp
{
    template<special_mem_req Req>
    struct allocation_box_type : fake_type_for<Req>
    {
    };
}

namespace stdsharp::details
{

    template<special_mem_req Req, allocator_req Alloc>
    struct box_allocation_value_traits
    {
        using allocation_traits = allocator_aware::allocation_traits<Alloc>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocation_type = allocation_traits::allocation_type;
        using allocator_type = allocation_traits::allocator_type;
        using fake_type = fake_type_for<Req>;

        using mov_dispatcher = dispatcher<
            get_expr_req(allocator_traits::template mov_constructible<fake_type>, allocator_traits::template nothrow_mov_constructible<fake_type>),
            void,
            const allocation_type&,
            const allocation_type&,
            allocator_type&>;

        using allocation_value =
            allocator_aware::allocation_value<Alloc, allocator_aware::allocation_dynamic_type<Req>>;

        using type = stdsharp::invocables<allocation_value, mov_dispatcher>;
    };

    template<special_mem_req Req, allocator_req Alloc>
    class box_allocation_value : stdsharp::details::box_allocation_value_traits<Req, Alloc>::type
    {
        using traits = stdsharp::details::box_allocation_value_traits<Req, Alloc>;

        template<special_mem_req, allocator_req>
        friend class box_allocation_value;

        std::string_view type_id_{};
        std::size_t type_size_{};

        using m_base = traits::type;

        using allocation_traits = allocator_aware::allocation_traits<Alloc>;
        using allocation_type = allocation_traits::allocation_type;
        using allocator_type = allocation_traits::allocator_type;
        using allocator_traits = allocation_traits::allocator_traits;

        template<typename T>
        struct mov_dispatcher
        {
            constexpr void operator()(
                const allocation_type src_allocation,
                const allocation_type dst_allocation,
                allocator_type& allocator
            ) const noexcept(allocator_traits::template nothrow_mov_constructible<T>)
                requires(allocator_traits::template mov_constructible<T>)
            {
                constexpr auto value_type_size = sizeof(allocator_type::value_type);
                constexpr auto t_size = sizeof(T);

                Expects(std::ranges::size(src_allocation) * value_type_size >= t_size);
                Expects(std::ranges::size(dst_allocation) * value_type_size >= t_size);

                allocator_traits::template construct<T>(
                    allocator,
                    allocation_data<T>(dst_allocation),
                    cpp_move(allocation_get<T>(src_allocation))
                );
            }
        };

        constexpr box_allocation_value(
            const m_base& base,
            const std::string_view type_id,
            const std::size_t type_size
        ) noexcept:
            m_base(base), type_id_(type_id), type_size_(type_size)
        {
        }

    public:
        box_allocation_value() = default;

        template<typename T>
            requires std::constructible_from<m_base, std::in_place_type_t<T>>
        explicit constexpr box_allocation_value(const std::in_place_type_t<T> tag) noexcept:
            box_allocation_value({tag, mov_dispatcher<T>{}}, type_id<T>, sizeof(T))
        {
        }

        template<special_mem_req OtherReq, typename Other = box_allocation_value<OtherReq, Alloc>>
            requires std::constructible_from<m_base, Other, Other> && (OtherReq != Req)
        explicit constexpr box_allocation_value(
            const box_allocation_value<OtherReq, Alloc>& other //
        ) noexcept:
            box_allocation_value({other, other}, other.type_id_, other.type_size_)
        {
        }

        [[nodiscard]] constexpr auto& type() const noexcept { return type_id_; }

        template<typename T>
        [[nodiscard]] constexpr auto is_type() const noexcept
        {
            return type_id_ == type_id<T>;
        }

        [[nodiscard]] constexpr bool empty() const noexcept { return type_id_.empty(); }

        [[nodiscard]] constexpr auto size() const noexcept { return type_size_; }
    };
}

namespace stdsharp::allocator_aware
{
    template<special_mem_req Req, allocator_req Alloc>
    class allocation_value<Alloc, allocation_box_type<Req>> :
        public stdsharp::details::box_allocation_value_traits<Req, Alloc>::impl
    {
        using traits = stdsharp::details::box_allocation_value_traits<Req, Alloc>;
        using m_base = typename traits::impl;

    public:
        using m_base::m_base;
    };
}