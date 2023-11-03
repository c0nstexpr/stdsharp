#pragma once

#include "allocation_value.h"

namespace stdsharp::details
{
    template<allocator_req Alloc>
    struct box_allocation_value_traits
    {
        using allocation_traits = allocation_traits<Alloc>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocation_type = allocation_traits::allocation_type;
        using allocator_type = allocation_traits::allocator_type;

        template<special_mem_req Req>
        struct req
        {
            using fake_type = fake_type_for<Req>;

            using mov_dispatcher = dispatcher<
                get_expr_req(allocator_traits::template mov_constructible<fake_type>, allocator_traits::template nothrow_mov_constructible<fake_type>),
                void,
                allocator_type&,
                const allocation_type&,
                const allocation_type&>;

            using allocation_value = allocation_value<Alloc, allocation_dynamic_type<Req>>;

            using type = stdsharp::invocables<allocation_value, mov_dispatcher>;
        };

        template<typename T>
        struct mov_dispatcher_fn
        {
            constexpr void operator()(
                allocator_type& allocator,
                const allocation_type src_allocation,
                const allocation_type dst_allocation
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
    };

    template<special_mem_req Req, allocator_req Alloc>
    class box_allocation_value :
        stdsharp::details::box_allocation_value_traits<Alloc>::template req<Req>::type
    {
        using traits = stdsharp::details::box_allocation_value_traits<Alloc>;

        using required = traits::template req<Req>;

        using m_base = required::type;

        template<special_mem_req, allocator_req>
        friend class box_allocation_value;

        std::string_view type_id_{};

        constexpr box_allocation_value(const m_base& base, const std::string_view type_id) noexcept:
            m_base(base), type_id_(type_id)
        {
        }

        constexpr const required::allocation_value& allocation_value() const noexcept
        {
            return *this;
        }

        constexpr const required::mov_dispatcher& mov_dispatcher() const noexcept { return *this; }

    public:
        box_allocation_value() = default;

        template<typename T, typename MovFn = traits::template mov_dispatcher_fn<T>>
            requires std::constructible_from<m_base, std::in_place_type_t<T>, MovFn>
        explicit constexpr box_allocation_value(const std::in_place_type_t<T> tag) noexcept:
            box_allocation_value({tag, MovFn{}}, type_id<T>)
        {
        }

        template<special_mem_req OtherReq, typename Other = box_allocation_value<OtherReq, Alloc>>
            requires std::constructible_from<m_base, Other, Other> && (OtherReq != Req)
        explicit constexpr box_allocation_value(
            const box_allocation_value<OtherReq, Alloc>& other //
        ) noexcept:
            box_allocation_value({other.allocation_value(), other.mov_dispatcher()}, other.type())
        {
        }

        [[nodiscard]] constexpr auto& type() const noexcept { return type_id_; }

        template<typename T>
        [[nodiscard]] constexpr auto is_type() const noexcept
        {
            return type_id_ == type_id<T>;
        }

        [[nodiscard]] constexpr bool empty() const noexcept { return type_id_.empty(); }

        [[nodiscard]] constexpr auto value_size() const noexcept
        {
            return cpo::get_element<0>(static_cast<const m_base&>(*this)).value_size();
        }
    };
}

namespace stdsharp
{
    template<special_mem_req Req>
    struct allocation_box_type : fake_type_for<Req>
    {
    };

    template<special_mem_req Req, allocator_req Alloc>
    class allocation_value<Alloc, allocation_box_type<Req>> :
        public stdsharp::details::box_allocation_value<Req, Alloc>
    {
        using m_base = stdsharp::details::box_allocation_value<Req, Alloc>;

    public:
        using m_base::m_base;
    };
}