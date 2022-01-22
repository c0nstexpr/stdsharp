//
// Created by BlurringShadow on 2021-10-20.
//

#pragma once
#include <array>

#include "functional/cpo.h"

namespace stdsharp::reflection
{
    using namespace ::std::literals;

    namespace details
    {
        template<::std::convertible_to<::std::string_view> auto Literal>
        struct member_t : functional::nodiscard_tag_t
        {
            static constexpr ::std::string_view name = Literal;
        };

        template<typename>
        struct is_std_pair;

        template<typename T, typename U>
        struct is_std_pair<::std::pair<T, U>>
        {
        };

        template<auto Literal>
        struct data_member_t : details::member_t<Literal>
        {
            template<typename T>
                requires requires
                {
                    requires static_cast<::std::string_view>(Literal) == "first";
                    is_std_pair<::std::remove_cvref_t<T>>{};
                }
            constexpr auto& operator()(T&& p) const noexcept { return ::std::forward<T>(p).first; }

            template<typename T>
                requires requires
                {
                    requires static_cast<::std::string_view>(Literal) == "second";
                    is_std_pair<::std::remove_cvref_t<T>>{};
                }
            constexpr auto& operator()(T&& p) const noexcept { return ::std::forward<T>(p).second; }
        };

        template<auto Literal>
        struct member_function_t : details::member_t<Literal>
        {
        };
    }

    template<auto Literal>
    using member_t = details::member_t<Literal>;

    template<auto Literal>
    inline constexpr functional::cpo_t<member_t<Literal>> member{};

    template<auto Literal>
    using data_member_t = details::data_member_t<Literal>;

    template<auto Literal>
    inline constexpr functional::cpo_t<data_member_t<Literal>> data_member{};

    template<auto Literal>
    using member_function_t = details::member_function_t<Literal>;

    template<auto Literal>
    inline constexpr functional::cpo_t<member_function_t<Literal>> member_function{};

    template<typename>
    struct data_members_t;

    template<typename T>
        requires requires { details::is_std_pair<T>{}; }
    struct data_members_t<T>
    {
        static constexpr ::std::array value = {"first"sv, "second"sv};
    };

    template<typename T>
    inline constexpr const auto& data_members = data_members_t<T>::value;
}
