// Created by BlurringShadow at 2021-03-05-下午 11:53

#pragma once

#include "concepts/concepts.h"
#include <utility>

namespace stdsharp::type_traits
{
    struct unique_object
    {
        unique_object() = default;
        unique_object(const unique_object&) = delete;
        constexpr unique_object(unique_object&&) noexcept {};
        unique_object& operator=(const unique_object&) = delete;
        constexpr unique_object& operator=(unique_object&&) noexcept { return *this; };
        ~unique_object() = default;
    };

    template<typename T>
    struct private_object
    {
        friend T;

    protected:
        private_object() noexcept = default;
        private_object(const private_object&) = default;
        private_object& operator=(const private_object&) = default;
        private_object(private_object&&) noexcept = default;
        private_object& operator=(private_object&&) noexcept = default;
        ~private_object() = default;
    };

    template<typename Base, typename... T>
    struct inherited : Base, T...
    {
        inherited() = default;

        template<typename... U>
            requires(
                ::std::constructible_from<Base, U...> && (::std::constructible_from<T> && ...) //
            )
        constexpr explicit inherited(U&&... u) //
            noexcept(
                concepts::nothrow_constructible_from<Base, U...> &&
                (concepts::nothrow_constructible_from<T> && ...) // clang-format off
            ):
            Base(::std::forward<U>(u)...) // clang-format on
        {
        }

        template<typename BaseT, typename... U>
            requires(
                ::std::constructible_from<Base, BaseT> &&
                (::std::constructible_from<T, U> && ...) //
            )
        constexpr explicit inherited(BaseT&& base, U&&... u) //
            noexcept(
                concepts::nothrow_constructible_from<Base, BaseT> &&
                (concepts::nothrow_constructible_from<T, U> && ...) // clang-format off
                ):
            Base(::std::forward<BaseT>(base)), T(::std::forward<U>(u))... // clang-format on
        {
        }

        template<typename U>
            requires ::std::assignable_from<Base, U>
        constexpr inherited& operator=(U&& u) noexcept(concepts::nothrow_assignable_from<Base, U>)
        {
            Base::operator=(::std::forward<U>(u));
            return *this;
        }
    };

    template<typename... T>
    inherited(T&&...) -> inherited<::std::decay_t<T>...>;

    struct make_inherited_fn
    {
        template<typename Base, typename... U>
            requires requires { inherited{::std::declval<Base>(), ::std::declval<U>()...}; }
        [[nodiscard]] constexpr auto operator()(Base&& base, U&&... u) const
            noexcept(noexcept(inherited{::std::declval<Base>(), ::std::declval<U>()...}))
        {
            return inherited{::std::forward<Base>(base), ::std::forward<U>(u)...};
        }
    };

    inline constexpr make_inherited_fn make_inherited{};
}
