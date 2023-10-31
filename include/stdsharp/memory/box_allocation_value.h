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
    struct box_allocation_value_traits_base
    {
    protected:
        struct direct_initialize_t
        {
            explicit constexpr direct_initialize_t() = default;
        };
    };

    template<special_mem_req Req, allocator_req Alloc>
    struct box_allocation_value_traits : box_allocation_value_traits_base
    {
        using allocation_traits = allocator_aware::allocation_traits<Alloc>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocation_type = allocation_traits::allocation_type;
        using allocator_type = allocation_traits::allocator_type;
        using direct_initialize_t = box_allocation_value_traits_base::direct_initialize_t;
        using fake_type = fake_type_for<Req>;

        using mov_dispatchers = dispatcher<
            get_expr_req(allocator_traits::template cp_constructible<fake_type>, allocator_traits::template nothrow_cp_constructible<fake_type>),
            void,
            const allocation_type&,
            const allocation_type&,
            allocator_type&>;

        using allocation_value =
            allocator_aware::allocation_value<Alloc, allocator_aware::allocation_dynamic_type<Req>>;

        using type = stdsharp::invocables<allocation_value, mov_dispatchers>;
    };
}

namespace stdsharp::allocator_aware
{
    template<special_mem_req Req, allocator_req Alloc>
    class allocation_value<Alloc, allocation_box_type<Req>> :
        stdsharp::details::box_allocation_value_traits<Req, Alloc>::type
    {
        using traits = stdsharp::details::box_allocation_value_traits<Req, Alloc>;

        using direct_initialize_t = typename traits::direct_initialize_t;

        std::string_view type_id_{};

        using m_base = traits::type;

        using allocation_traits = allocation_traits<Alloc>;
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

    public:
        allocation_value() = default;

        constexpr allocation_value(
            const direct_initialize_t /*unused*/,
            const m_base& base,
            const std::string_view type_id
        ) noexcept:
            m_base(base), type_id_(type_id)
        {
        }

        template<typename T>
            requires std::constructible_from<m_base, std::in_place_type_t<T>>
        explicit constexpr allocation_value(const std::in_place_type_t<T> tag) noexcept:
            m_base(tag, mov_dispatcher<T>{}), type_id_(type_id<T>)
        {
        }

        template<
            special_mem_req OtherReq,
            typename Other = allocation_value<Alloc, allocation_box_type<OtherReq>>>
            requires requires(
                traits::allocation_value base_dispatcher,
                traits::mov_dispatcher mov_dispatcher
            ) {
                requires(Req != OtherReq);
                Other{direct_initialize_t{}, {base_dispatcher, mov_dispatcher}, std::string_view{}};
            }
        explicit constexpr operator allocation_value<Alloc, allocation_box_type<OtherReq>>()
        {
            const auto& base_dispatcher = this->template get<0>();
            const auto& mov_dispatcher = this->template get<1>();
            return {direct_initialize_t{}, {base_dispatcher, mov_dispatcher}, type()};
        }

        [[nodiscard]] constexpr auto& type() const noexcept { return type_id_; }

        template<typename T>
        [[nodiscard]] constexpr auto is_type() const noexcept
        {
            return type_id_ == type_id<T>;
        }

        [[nodiscard]] constexpr bool empty() const noexcept { return type_id_.empty(); }
    };
}