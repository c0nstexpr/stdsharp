// Created by BlurringShadow at 2021-03-01-下午 9:00

#pragma once

#include <type_traits>

#include <range/v3/functional/arithmetic.hpp>

namespace stdsharp::type_traits
{
    inline constexpr struct empty_t final
    {
    } empty;

    template<typename T>
    inline constexpr ::std::type_identity<T> type_identity_v{};

    template<typename T>
    using add_const_lvalue_ref_t = ::std::add_lvalue_reference_t<::std::add_const_t<T>>;

    template<auto Value>
    struct constant
    {
        using value_type = decltype(Value);

        static constexpr auto value = Value;

        explicit constexpr operator const value_type&() const noexcept { return value; }

        [[nodiscard]] constexpr auto& operator()() const noexcept { return value; }
    };

    template<auto Value>
    inline constexpr auto constant_v = Value;

    template<typename T>
    constexpr const auto& static_const_v = ::ranges::static_const<T>::value;

    template<bool conditional, auto left, auto right>
    inline constexpr auto conditional_v = ::std::conditional_t<
        conditional,
        type_traits::constant<left>,
        type_traits::constant<right> // clang-format off
    >::value; // clang-format on

    template<typename T>
    concept constant_value = requires
    {
        std::bool_constant<(std::declval<T>().value, true)>{};
    };

    template<std::size_t I>
    using index_constant = ::std::integral_constant<::std::size_t, I>;

    template<typename T>
    struct type_constant : ::std::type_identity<T>
    {
        template<template<typename> typename OtherT, typename U>
        friend constexpr auto operator==(const type_constant, const OtherT<U>&) noexcept
        {
            return ::std::same_as<T, U>;
        }

        template<template<typename> typename OtherT, typename U>
        friend constexpr auto operator!=(const type_constant l, const OtherT<U>& r) noexcept
        {
            return !(l == r);
        }
    };

    template<typename T>
    inline constexpr type_constant<T> type_constant_v{};

    template<typename T>
    using coerce_t = ::std::invoke_result_t<::ranges::coerce<T>, T&&>;
}
