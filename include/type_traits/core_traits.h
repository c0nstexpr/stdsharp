// Created by BlurringShadow at 2021-03-01-下午 9:00

#pragma once

#include <array>
#include <type_traits>
#include <string_view>

#include <range/v3/functional/arithmetic.hpp>
#include <range/v3/utility/static_const.hpp>

namespace stdsharp::type_traits
{
    inline constexpr struct empty_t final
    {
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr empty_t(const auto&...) noexcept {}
    } empty;

    template<typename T>
    inline constexpr ::std::type_identity<T> type_identity_v{};

    template<typename T>
    using add_const_lvalue_ref_t = ::std::add_lvalue_reference_t<::std::add_const_t<T>>;

    template<auto Value>
    using constant = ::std::integral_constant<decltype(Value), Value>;

    template<auto Func, auto... Args>
        requires ::std::invocable<decltype(Func), decltype(Args)...>
    static constexpr decltype(auto) invoke_result = ::std::invoke(Func, Args...);

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
        ::std::bool_constant<(::std::declval<T>().value, true)>{};
    };

    template<::std::size_t I>
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

    template<auto... V>
    struct regular_value_sequence
    {
        static constexpr auto size() noexcept { return sizeof...(V); }
    };

    template<typename T, typename U>
    using ref_align_t = ::std::conditional_t<
        ::std::is_lvalue_reference_v<T>,
        ::std::add_lvalue_reference_t<U>, // clang-format off
        ::std::conditional_t<::std::is_rvalue_reference_v<T>, ::std::add_rvalue_reference_t<U>, U>
    >; // clang-format on

    template<typename T, typename U>
    using const_align_t = ::std::conditional_t<
        ::std::is_const_v<::std::remove_reference_t<T>>,
        ::std::add_const_t<U>, // clang-format off
        U
    >; // clang-format on

    template<typename T, typename U>
    using const_ref_align_t = ref_align_t<T, const_align_t<T, U>>;

    template<typename... T>
    using regular_type_sequence = ::meta::list<T...>;

    inline namespace literals
    {
        template<::std::size_t Size>
        struct ltr : ::std::array<char, Size>
        {
            using base = ::std::array<char, Size>;
            using base::base;

            // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays,hicpp-explicit-conversions)
            constexpr ltr(const char (&arr)[Size]) noexcept: base(::std::to_array(arr)) {}

            // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
            constexpr ltr& operator=(const char (&arr)[Size]) noexcept
            {
                *this = ::std::to_array(arr);
            }

            // NOLINTNEXTLINE(hicpp-explicit-conversions)
            constexpr operator ::std::string_view() const noexcept
            {
                return {base::data(), Size - 1};
            }

            constexpr auto to_string_view() const noexcept
            {
                return static_cast<::std::string_view>(*this); //
            }
        };

        template<ltr ltr>
        constexpr auto operator"" _ltr() noexcept
        {
            return ltr;
        }
    }
}

namespace meta::extension
{
    template<invocable Fn, template<auto...> typename T, auto... V>
    struct apply<Fn, T<V...>> : lazy::invoke<Fn, ::stdsharp::type_traits::constant<V>...>
    {
    };
}

namespace stdsharp::inline literals
{
    using namespace stdsharp::type_traits::literals;
}