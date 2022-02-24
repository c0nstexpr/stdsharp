//
// Created by BlurringShadow on 2021-10-20.
//

#pragma once
#include <array>
#include <string_view>

#include "functional/cpo.h"

namespace stdsharp::reflection
{
    using namespace ::std::literals;

    namespace details
    {
        template<::std::convertible_to<::std::string_view> auto Literal>
        struct member_t
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
    struct member_t
    {
        template<typename... Args>
            requires(
                ::std::invocable<details::member_t<Literal>, Args...> &&
                !functional::cpo_invocable<member_t<Literal>, Args...>)
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(concepts::nothrow_invocable<details::member_t<Literal>, Args...>)
        {
            return details::member_t<Literal>{}(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires functional::cpo_invocable<member_t<Literal>, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(functional::cpo_nothrow_invocable<member_t<Literal>, Args...>)
        {
            return functional::cpo_invoke(*this, ::std::forward<Args>(args)...);
        }
    };

    template<auto Literal>
    inline constexpr member_t<Literal> member{};

    template<auto Literal>
    struct data_member_t
    {
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr data_member_t(const member_t<Literal> = {}) noexcept {}

        template<typename... Args>
            requires(
                ::std::invocable<details::data_member_t<Literal>, Args...> &&
                !functional::cpo_invocable<data_member_t<Literal>, Args...>)
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(concepts::nothrow_invocable<details::data_member_t<Literal>, Args...>)
        {
            return details::data_member_t<Literal>{}(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires functional::cpo_invocable<data_member_t<Literal>, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(functional::cpo_nothrow_invocable<data_member_t<Literal>, Args...>)
        {
            return functional::cpo_invoke(*this, ::std::forward<Args>(args)...);
        }
    };

    template<auto Literal>
    inline constexpr data_member_t<Literal> data_member{};

    template<auto Literal>
    struct member_function_t : member_t<Literal>
    {
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr member_function_t(const member_t<Literal> = {}) noexcept {}

        template<typename... Args>
            requires(
                ::std::invocable<details::member_function_t<Literal>, Args...> &&
                !functional::cpo_invocable<member_function_t<Literal>, Args...>)
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(concepts::nothrow_invocable<details::member_function_t<Literal>, Args...>)
        {
            return details::member_function_t<Literal>{}(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires functional::cpo_invocable<member_function_t<Literal>, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(functional::cpo_nothrow_invocable<member_function_t<Literal>, Args...>)
        {
            return functional::cpo_invoke(*this, ::std::forward<Args>(args)...);
        }
    };

    template<auto Literal>
    inline constexpr member_function_t<Literal> member_function{};

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
