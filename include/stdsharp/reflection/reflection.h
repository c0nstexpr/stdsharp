//
// Created by BlurringShadow on 2021-10-20.
//

#pragma once

#include <array>
#include <string_view>

using namespace ::std::literals;

namespace stdsharp::reflection
{
    namespace details
    {
        template<typename>
        struct is_std_pair;

        template<typename T, typename U>
        struct is_std_pair<::std::pair<T, U>>
        {
        };

        template<auto V>
        void member(auto&&) = delete;

        template<::std::convertible_to<::std::string_view> auto Literal>
        struct member_t
        {
            static constexpr ::std::string_view name = Literal;

            template<typename T>
                requires requires { member<Literal>(::std::declval<T>()); }
            constexpr decltype(auto) operator()(T&& t) const
                noexcept(noexcept(member<Literal>(::std::declval<T>())))
            {
                return member<Literal>(::std::forward<T>(t));
            }
        };

        template<auto V>
        void data_member(auto&&) = delete;

        template<auto Literal>
        struct data_member_t : member_t<Literal>
        {
            constexpr data_member_t(const member_t<Literal> = {}) noexcept {}

            template<typename T>
                requires requires
                {
                    requires static_cast<::std::string_view>(Literal) == "first";
                    details::is_std_pair<::std::remove_cvref_t<T>>{};
                }
            constexpr auto& operator()(T&& p) const noexcept { return ::std::forward<T>(p).first; }

            template<typename T>
                requires requires
                {
                    requires static_cast<::std::string_view>(Literal) == "second";
                    details::is_std_pair<::std::remove_cvref_t<T>>{};
                }
            constexpr auto& operator()(T&& p) const noexcept { return ::std::forward<T>(p).second; }

            template<typename T>
                requires requires { data_member<Literal>(::std::declval<T>()); }
            constexpr decltype(auto) operator()(T&& t) const
                noexcept(noexcept(data_member<Literal>(::std::declval<T>())))
            {
                return data_member<Literal>(::std::forward<T>(t));
            }
        };

        template<auto V>
        void member_function(auto&&) = delete;

        template<auto Literal>
        struct member_function_t : member_t<Literal>
        {
            constexpr member_function_t(const member_t<Literal> = {}) noexcept {}

            template<typename T>
                requires requires { member_function<Literal>(::std::declval<T>()); }
            constexpr decltype(auto) operator()(T&& t) const
                noexcept(noexcept(member_function<Literal>(::std::declval<T>())))
            {
                return member_function<Literal>(::std::forward<T>(t));
            }
        };
    }

    inline namespace cpo
    {
        using details::member_t;
        using details::data_member_t;
        using details::member_function_t;

        template<auto Literal>
        inline constexpr member_t<Literal> member{};

        template<auto Literal>
        inline constexpr data_member_t<Literal> data_member{};

        template<auto Literal>
        inline constexpr member_function_t<Literal> member_function{};
    }

    template<typename>
    inline constexpr int data_members = 0;

    template<typename T>
        requires requires { details::is_std_pair<T>{}; }
    inline constexpr ::std::array data_members<T> = {"first"sv, "second"sv};
}