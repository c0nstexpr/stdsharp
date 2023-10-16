#pragma once

#include "details/box_allocation.h"
#include "allocator_aware/basic_aa.h"
#include "../type_traits/object.h"

namespace stdsharp
{
    template<special_mem_req Req, allocator_req Alloc>
    class box : allocator_aware::basic_aa<details::box_allocation<Req, Alloc>>
    {
        template<special_mem_req, allocator_req>
        friend class box;

    public:
        using allocator_type = Alloc;

    private:
        using m_base = basic_allocator_aware<box<Req, allocator_type>, allocator_type>;
        template<typename T>
        using typed_allocation = m_base::template typed_allocation<T>;
        using dispatchers = details::box_dispatchers<Req, allocator_type>;

    public:
        static constexpr auto req = dispatchers::req;

        using m_base::m_base;
        box() = default;

        template<typename... T>
        constexpr box(
            const std::allocator_arg_t /*unused*/,
            const allocator_type& alloc,
            T&&... args
        ) noexcept(nothrow_constructible_from<box, T..., const allocator_type&>)
            requires std::is_constructible_v<box, T..., const allocator_type&>
            : box(cpp_forward<T>(args)..., alloc)
        {
        }

        template<
            typename... Args,
            typename T,
            typename ValueType = std::decay_t<T>,
            typename Identity = std::type_identity<ValueType> // clang-format off
        > // clang-format on
        constexpr box(
            const std::allocator_arg_t /*unused*/,
            const allocator_type& alloc,
            const std::in_place_type_t<T> /*unused*/,
            Args&&... args
        )
            requires requires {
                requires std::constructible_from<compressed_t, Identity, const allocator_type&>;
                m_base::template construct<ValueType>(cpp_forward(args)...);
            }
            :
            compressed_(Identity{}, alloc),
            allocation_(m_base::template construct<ValueType>(cpp_forward(args)...))
        {
        }

        template<
            typename... Args,
            typename T,
            typename ValueType = std::decay_t<T>,
            typename Identity = std::type_identity<ValueType> // clang-format off
        > // clang-format on
        constexpr box(const std::in_place_type_t<T> /*unused*/, Args&&... args)
            requires requires {
                requires std::constructible_from<compressed_t, Identity>;
                m_base::template construct<ValueType>(cpp_forward(args)...);
            }
            :
            compressed_(Identity{}),
            allocation_(m_base::template construct<ValueType>(cpp_forward(args)...))
        {
        }

        template<special_mem_req OtherReq, typename OtherBox = box<OtherReq, allocator_type>>
            requires OtherBox::faked_typed::cp_constructible
        constexpr box(const box<OtherReq, allocator_type>& other, const allocator_type& alloc):
            compressed_(other.get_dispatchers(), alloc),
            allocation_(
                get_dispatchers() ?
                    get_dispatchers().construct(get_allocator(), other.get_allocation()) :
                    allocation{}
            )
        {
        }

        template<special_mem_req OtherReq, typename OtherBox = box<OtherReq, allocator_type>>
            requires std::is_constructible_v<box, const OtherBox&, const allocator_type&>
        explicit(OtherReq != Req) constexpr box(const box<OtherReq, allocator_type>& other):
            compressed_(
                other.get_dispatchers(),
                select_on_container_copy_construction(other.get_allocator())
            ),
            allocation_(
                get_dispatchers() ?
                    get_dispatchers().construct(get_allocator(), other.get_allocation()) :
                    allocation{}
            )
        {
        }

        template<special_mem_req OtherReq, typename OtherBox = box<OtherReq, allocator_type>>
            requires OtherBox::faked_typed::mov_constructible
        constexpr box(box<OtherReq, allocator_type>&& other, const allocator_type& alloc) //
            noexcept(is_noexcept(OtherBox::req.move_construct)):
            compressed_(other.get_dispatchers(), alloc),
            allocation_(
                get_dispatchers() ?
                    get_dispatchers().construct(get_allocator(), cpp_move(other).get_allocation()) :
                    allocation{}
            )
        {
        }

        template<special_mem_req OtherReq, typename OtherBox = box<OtherReq, allocator_type>>
            requires OtherBox::faked_typed::mov_constructible
        explicit(OtherReq != Req) constexpr box(box<OtherReq, allocator_type>&& other) //
            noexcept(is_noexcept(OtherBox::req.move_construct)):
            compressed_(other.get_dispatchers(), cpp_move(other).get_allocator()),
            allocation_(
                get_dispatchers() ?
                    get_dispatchers().construct(get_allocator(), cpp_move(other).get_allocation()) :
                    allocation{}
            )
        {
        }

        [[nodiscard]] constexpr operator bool() const noexcept
        {
            return get_dispatchers().has_value();
        }

        constexpr void destroy() noexcept(is_noexcept(req.destruct))
            requires faked_typed::destructible
        {
            auto& dispatchers = get_dispatchers();

            if(!dispatchers) return;

            dispatchers.destroy(get_allocator(), get_allocation(), true);
            dispatchers = {};
        }

        [[nodiscard]] constexpr const allocator_type& get_allocator() const noexcept
        {
            return cpo::get_element<1>(compressed_);
        }

    private:
        [[nodiscard]] constexpr dispatchers& get_dispatchers() noexcept
        {
            return cpo::get_element<0>(compressed_);
        }

        [[nodiscard]] constexpr const dispatchers& get_dispatchers() const noexcept
        {
            return cpo::get_element<0>(compressed_);
        }

        [[nodiscard]] constexpr auto& get_allocation() noexcept { return allocation_; }

        [[nodiscard]] constexpr auto& get_allocation() const noexcept { return allocation_; }

        [[nodiscard]] constexpr allocator_type& get_allocator() noexcept
        {
            return cpo::get_element<1>(compressed_);
        }

        template<typename T, typename... Args>
        static constexpr auto emplace_constructible =
            m_base::template constructible_from<T, Args...> &&
            std::constructible_from<dispatchers, std::type_identity<T>> &&
            faked_typed::destructible;

    public:
        template<std::same_as<void> T = void>
            requires faked_typed::destructible
        constexpr void emplace() noexcept(noexcept(destroy()))
        {
            destroy();
        }

        template<typename T, typename... Args>
            requires emplace_constructible<T, Args...>
        constexpr decltype(auto) emplace(Args&&... args)
        {
            destroy();
            get_allocation().allocate(get_allocator(), sizeof(T));

            typed_allocation<T> allocation{get_allocation()};

            allocation.construct(get_allocator(), cpp_forward(args)...);
            get_dispatchers() = dispatchers{std::type_identity<T>{}};

            return get<T>();
        }

        template<typename T, typename... Args, typename U>
        constexpr decltype(auto) emplace(const std::initializer_list<U> il, Args&&... args)
            requires emplace_constructible<T, decltype(il), Args...>
        {
            return emplace<T, decltype(il), Args...>(il, cpp_forward(args)...);
        }

        template<typename T>
            requires emplace_constructible<std::decay_t<T>, T>
        constexpr decltype(auto) emplace(T&& t)
        {
            return emplace<std::decay_t<T>, T>(cpp_forward(t));
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

        template<typename T>
            requires std::constructible_from<dispatchers, std::type_identity<T>>
        [[nodiscard]] constexpr auto is_type() const noexcept
        {
            return get_dispatchers() == dispatchers{std::type_identity<T>{}};
        }

        template<typename>
        [[nodiscard]] constexpr auto is_type() const noexcept
        {
            return false;
        }

        [[nodiscard]] constexpr auto reserved() const noexcept { return get_allocation().size(); }
    }; // NOLINTEND(*-noexcept-*)

    template<typename T, allocator_req Alloc>
    using box_for = box<special_mem_req::for_type<T>(), Alloc>;

    template<allocator_req Alloc>
    using trivial_box = box_for<trivial_object, Alloc>;

    template<allocator_req Alloc>
    using normal_box = box_for<normal_object, Alloc>;

    template<allocator_req Alloc>
    using unique_box = box_for<unique_object, Alloc>;
}