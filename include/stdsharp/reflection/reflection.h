//
// Created by BlurringShadow on 2021-10-20.
//

#pragma once

#include <ranges>

#include "../functional/invocables.h"

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

        friend bool operator==(member_info, member_info) = default;
        friend bool operator!=(member_info, member_info) = default;
    };

    template<typename T>
    struct get_members_t;

    template<typename T, typename U>
    struct get_members_t<::std::pair<T, U>>
    {
        [[nodiscard]] constexpr auto operator()() const noexcept
        {
            return ::std::array{
                member_info{"first", member_category::data},
                member_info{"second", member_category::data} //
            };
        }
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

        template<auto Literal>
        struct get_member_fn
        {
            static constexpr ::std::string_view name = Literal;

            template<typename T>
                requires requires { get_member<Literal>(::std::declval<T>()); }
            [[nodiscard]] constexpr decltype(auto) operator()(T&& t) const
                noexcept(noexcept(get_member<Literal>(::std::declval<T>())))
            {
                return get_member<Literal>(::std::forward<T>(t));
            }

            template<typename T>
                requires requires //
            {
                requires name == "first";
                is_std_pair<::std::remove_cvref_t<T>>{};
            }
            [[nodiscard]] constexpr auto& operator()(T&& p) const noexcept
            {
                return ::std::forward<T>(p).first;
            }

            template<typename T>
                requires requires //
            {
                requires name == "second";
                is_std_pair<::std::remove_cvref_t<T>>{};
            }
            [[nodiscard]] constexpr auto& operator()(T&& p) const noexcept
            {
                return ::std::forward<T>(p).second;
            }
        };

        template<typename T>
        struct get_members_fn
        {
            [[nodiscard]] constexpr ::std::ranges::output_range<member_info> auto
                operator()() const noexcept
                requires(noexcept(reflection::get_members_t<T>{}()))
            {
                return reflection::get_members_t<T>{}();
            }
        };
    }

    inline namespace cpo
    {
        using details::get_member_fn;
        using details::get_members_fn;

        template<type_traits::ltr Literal>
        inline constexpr get_member_fn<Literal> get_member{};

        template<typename T>
        inline constexpr get_members_fn<T> get_members{};
    }

    template<typename T>
    inline constexpr functional::nodiscard_invocable get_data_members{
        []() noexcept
        {
            return get_members<T>() |
                ::std::views::filter( // clang-format off
                    [](const member_info info)
                    {
                        return info.category == member_category::data;
                    }
                ); // clang-format on
        } //
    };
}