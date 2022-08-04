//
// Created by BlurringShadow on 2021-10-20.
//

#pragma once

#include <array>
#include <string_view>

#include "../type_traits/core_traits.h"

using namespace ::std::literals;

namespace stdsharp::reflection
{

    enum class member_category
    {
        function,
        data
    };

    struct member_info
    {
        ::std::string_view name{};
        member_category category{};
    };

    namespace details
    {
        template<typename>
        struct is_std_pair;

        template<typename T, typename U>
        struct is_std_pair<::std::pair<T, U>>
        {
        };

        template<auto V>
        void get_member(auto&&) = delete;

        template<::std::convertible_to<::std::string_view> auto Literal>
        struct get_member_t
        {
            static constexpr ::std::string_view name = Literal;

            template<typename T>
                requires requires { get_member<Literal>(::std::declval<T>()); }
            constexpr decltype(auto) operator()(T&& t) const
                noexcept(noexcept(get_member<Literal>(::std::declval<T>())))
            {
                return get_member<Literal>(::std::forward<T>(t));
            }

            template<typename T>
                requires requires
                {
                    requires name == "first";
                    details::is_std_pair<::std::remove_cvref_t<T>>{};
                }
            constexpr auto& operator()(T&& p) const noexcept { return ::std::forward<T>(p).first; }

            template<typename T>
                requires requires
                {
                    requires name == "second";
                    details::is_std_pair<::std::remove_cvref_t<T>>{};
                }
            constexpr auto& operator()(T&& p) const noexcept { return ::std::forward<T>(p).second; }
        };

        template<typename T>
        void get_members() = delete;

        template<typename T>
            requires requires { details::is_std_pair<::std::remove_cvref_t<T>>{}; }
        constexpr auto get_members() noexcept
        {
            return ::std::array{
                member_info{"first", member_category::data},
                member_info{"second", member_category::data} //
            };
        }

        template<typename T>
        struct get_members_t
        {
            constexpr ::std::ranges::output_range<member_info> auto operator()() const noexcept
                requires(noexcept(get_member<T>()))
            {
                return get_member<T>();
            }
        };
    }

    inline namespace cpo
    {
        using details::get_member_t;
        using details::get_members_t;

        template<::std::convertible_to<::std::string_view> auto Literal>
        inline constexpr get_member_t<Literal> get_member{};

        template<typename T>
        inline constexpr get_members_t<T> get_members{};

    }
}