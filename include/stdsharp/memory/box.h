#pragma once

#include "details/box_dispatchers.h"
#include "../type_traits/object.h"

namespace stdsharp
{
    namespace details
    {
        template<special_mem_req Req, allocator_req Alloc>
        class box : public basic_allocator_aware<box<Req, Alloc>, Alloc>
        {
        public:
            using allocator_type = Alloc;

        private:
            using traits = allocator_aware_traits<allocator_type>;
            using m_base = basic_allocator_aware<box<Req, allocator_type>, allocator_type>;
            using dispatchers = box_dispatchers<Req, allocator_type>;
            using typename traits::allocation;
            using compressed_t = stdsharp::indexed_values<dispatchers, allocator_type>;

            compressed_t compressed_{};
            allocation allocation_{};

        public:
            using m_base::m_base;
            box() = default;
            box(const box&) = default;
            box(box&&) = default;

            constexpr box(const allocator_type& alloc): compressed_(dispatchers{}, alloc) {}

            template<
                typename... Args,
                typename T,
                typename ValueType = std::decay_t<T>,
                typename Identity = std::type_identity<ValueType> // clang-format off
            > // clang-format on
                requires std::constructible_from<compressed_t, Identity, const allocator_type&> &&
                             std::invocable<make_typed_allocation_fn<T>, allocator_type&, Args...>
            constexpr box(
                const std::allocator_arg_t,
                const allocator_type& alloc,
                const std::in_place_type_t<T>,
                Args&&... args
            ):
                compressed_(Identity{}, alloc),
                allocation_(make_typed_allocation<T>(get_allocator(), cpp_forward(args)...))
            {
            }

            template<special_mem_req OtherReq>
                requires((OtherReq >= Req) && (Req.copy_construct >= expr_req::well_formed))
            constexpr box(const box<OtherReq, allocator_type>& other, const allocator_type& alloc):
                compressed_(other.get_dispatchers(), alloc),
                allocation_(
                    get_dispatchers() ?
                        get_dispatchers().construct(get_allocator(), other.get_allocation()) :
                        allocation{}
                )
            {
            }

            template<special_mem_req OtherReq>
                requires(OtherReq >= Req)
            constexpr box(
                box<OtherReq, allocator_type>&& other,
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
            template<special_mem_req OtherReq>
                requires((OtherReq >= Req) && (Req.copy_assign >= expr_req::well_formed) && ())
            constexpr box& operator=(const box<OtherReq, allocator_type>& other) //
                noexcept(Req.copy_assign >= expr_req::no_exception)
            {
                return assign_impl(other);
            }

            box& operator=(box&)
                requires false;

            template<special_mem_req OtherReq>
                requires((OtherReq >= Req) && (Req.move_assign >= expr_req::well_formed))
            constexpr box& operator=(box<OtherReq, allocator_type>&& other) //
                noexcept(traits::propagate_on_move_v || traits::always_equal_v)
            {
                return assign_impl(other);
            }

            box& operator=(box&&) noexcept
                requires false;

            constexpr ~box()
            {
                destroy();
                get_allocation().deallocate(get_allocator());
            }

            template<special_mem_req OtherReq>
                requires(OtherReq >= Req)
            constexpr void swap(box<OtherReq, allocator_type>& other) noexcept
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

            [[nodiscard]] constexpr allocator_type& get_allocator() const noexcept
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

        private:
            [[nodiscard]] constexpr auto& get_dispatchers() noexcept
            {
                return stdsharp::get<0>(compressed_);
            }

            [[nodiscard]] constexpr auto& get_dispatchers() const noexcept
            {
                return stdsharp::get<0>(compressed_);
            }

            [[nodiscard]] constexpr allocator_type& get_allocator() noexcept
            {
                return stdsharp::get<1>(compressed_);
            }

            [[nodiscard]] constexpr auto& get_allocation() noexcept { return allocation_; }

            [[nodiscard]] constexpr auto& get_allocation() const noexcept { return allocation_; }

            using this_t = box;

        public:
            template<typename T, typename... Args>
            static constexpr auto emplace_constructible =
                m_base::template constructible_from<std::decay_t<T>, Args...> &&
                std::constructible_from<dispatchers, std::type_identity<std::decay_t<T>>>;

            static constexpr auto req = Req;

            template<std::same_as<void> T = void>
            constexpr void emplace() noexcept
            {
                destroy();
            }

            template<typename T, typename... Args>
                requires emplace_constructible<T, Args...>
            constexpr decltype(auto) emplace(Args&&... args)
            {
                using value_t = std::decay_t<T>;

                destroy();
                get_allocation().allocate(get_allocator(), sizeof(value_t));

                this_t::construct(get_allocator(), ptr<value_t>(), cpp_forward(args)...);
                get_dispatchers() = dispatchers{std::type_identity<value_t>{}};

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

            [[nodiscard]] constexpr auto type() const noexcept { return get_dispatchers().type(); }

            [[nodiscard]] constexpr auto size() const noexcept { return get_dispatchers().size(); }

            [[nodiscard]] constexpr auto reserved() const noexcept
            {
                return get_allocation().size();
            }
        };
    }

    template<special_mem_req Req, allocator_req Alloc>
    using box = details::box<Req, Alloc>;

    template<typename T, allocator_req Alloc>
    using box_for = box<allocation_constraints<Alloc, T>, Alloc>;

    template<allocator_req Alloc>
    using trivial_box = box_for<trivial_object, Alloc>;

    template<allocator_req Alloc>
    using normal_box = box_for<normal_object, Alloc>;

    template<allocator_req Alloc>
    using unique_box = box_for<unique_object, Alloc>;
}