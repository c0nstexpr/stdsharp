#pragma once

#include <algorithm>
#include <string_view>

#include "../concepts/concepts.h"
#include "pointer_traits.h"

namespace stdsharp
{
    struct special_member_requirement
    {
        enum level : unsigned char
        {
            ill_formed,
            well_formed,
            no_exception,
        };

    private:
        using partial_ordering = ::std::partial_ordering;

        struct spec
        {
            level movable;
            level copyable;

        private:
            friend constexpr partial_ordering
                operator<=>(const spec left, const spec right) noexcept
            {
                const auto movable_comp = left.movable <=> right.movable;

                return movable_comp == (left.copyable <=> right.copyable) ?
                    movable_comp :
                    partial_ordering::unordered;
            }

            friend constexpr spec min(const spec left, const spec right) noexcept
            {
                return {
                    ::std::min(left.movable, right.movable),
                    ::std::min(left.copyable, right.copyable) //
                };
            }
        };

        friend constexpr partial_ordering operator<=>(
            const special_member_requirement left,
            const special_member_requirement right
        ) noexcept
        {
            const auto construct_comp = left.construct <=> right.construct;

            return construct_comp == (left.assign <=> right.assign) ? construct_comp :
                                                                      partial_ordering::unordered;
        }

        friend constexpr auto operator==(
            const special_member_requirement left,
            const special_member_requirement right
        ) noexcept
        {
            return (left <=> right) == partial_ordering::equivalent;
        }

        friend constexpr special_member_requirement min( //
            const special_member_requirement left,
            const special_member_requirement right
        ) noexcept
        {
            return {min(left.construct, right.construct), min(left.assign, right.assign)};
        }

    public:
        spec construct;
        spec assign;

        template<typename T>
        [[nodiscard]] static constexpr special_member_requirement from() noexcept
        {
            return {
                {
                    .movable = ::std::move_constructible<T> ?
                        (nothrow_move_constructible<T> ? spec::no_exception : spec::well_formed) :
                        spec::ill_formed,
                    .copyable = ::std::copy_constructible<T> ?
                        (nothrow_copy_constructible<T> ? spec::no_exception : spec::well_formed) :
                        spec::ill_formed //
                },
                {
                    .movable = move_assignable<T> ?
                        (nothrow_move_assignable<T> ? spec::no_exception : spec::well_formed) :
                        spec::ill_formed,
                    .copyable = copy_assignable<T> ?
                        (nothrow_copy_assignable<T> ? spec::no_exception : spec::well_formed) :
                        spec::ill_formed //
                } //
            };
        }
    };

    enum class special_mem_type
    {
        move_construct,
        copy_construct,
        move_assign,
        copy_assign,
        destroy
    };

    namespace details
    {
        template<special_member_requirement::level Level, special_mem_type Mem, typename... Ptrs>
        struct special_mem_ptr
        {
            static constexpr auto is_noexcept =
                Level == special_member_requirement::level::no_exception;

            using ptr_t = void (*)(Ptrs...) noexcept(is_noexcept);

            constexpr special_mem_ptr(const ptr_t value) noexcept: value(value) {}

            constexpr void operator()(const constant<Mem>, const Ptrs&... args) const
                noexcept(is_noexcept)
            {
                (*value)(args...);
            }

        private:
            ptr_t value;
        };

        template<special_mem_type Mem, typename... Args>
        struct special_mem_ptr<special_member_requirement::level::ill_formed, Mem, Args...>
        {
        };

        struct type_info
        {
        private:
            ::std::string_view curent_type_{};
            ::std::size_t type_size_{};

        public:
            template<typename T>
            constexpr type_info(const ::std::type_identity<T>) noexcept:
                curent_type_(type_id<T>), type_size_(sizeof(T))
            {
            }

            [[nodiscard]] constexpr auto type() const noexcept { return curent_type_; }

            [[nodiscard]] constexpr auto size() const noexcept { return type_size_; }
        };

        template<special_member_requirement requirement, typename Ptr>
        struct special_member_traits
        {
            using pointer_traits = pointer_traits<Ptr>;

            static constexpr auto req_construct = requirement.construct;
            static constexpr auto req_assign = requirement.assign;
            using mem_type = special_mem_type;
            using mem_level = special_member_requirement::level;

            template<
                typename ConstPtr = typename pointer_traits:: //
                template rebind<const typename pointer_traits::element_type> // clang-format off
            > // clang-format on
            struct impl :
                type_info,
                special_mem_ptr<req_construct.movable, mem_type::move_construct, Ptr, Ptr>,
                special_mem_ptr<req_construct.copyable, mem_type::copy_construct, Ptr, ConstPtr>,
                special_mem_ptr<req_assign.movable, mem_type::move_assign, Ptr, Ptr>,
                special_mem_ptr<req_assign.copyable, mem_type::copy_assign, Ptr, ConstPtr>,
                special_mem_ptr<mem_level::no_exception, mem_type::destroy, Ptr>
            {
            };
        };
    }
}