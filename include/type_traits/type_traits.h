// Created by BlurringShadow at 2021-03-01-下午 9:00

#pragma once

#include <type_traits>

#include <range/v3/functional/arithmetic.hpp>
#include <range/v3/utility/static_const.hpp>

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

    template<auto...>
    struct regular_value_sequence
    {
    };

    template<auto...>
    struct value_sequence;

    template<typename>
    struct take_value_sequence;

    template<template<auto...> typename T, auto... Values>
    struct take_value_sequence<T<Values...>>
    {
        template<template<auto...> typename U>
        using apply_t = U<Values...>;

        using as_sequence_t = regular_value_sequence<Values...>;

        using as_value_sequence_t = value_sequence<Values...>;
    };

    template<typename Sequence>
    using to_regular_value_sequence_t = typename take_value_sequence<Sequence>::as_vsequence_t;

    template<typename Sequence> // clang-format off
    using to_value_sequence_t = typename
        take_value_sequence<Sequence>::as_value_sequence_t; // clang-format on

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

    template<typename...>
    struct regular_type_sequence
    {
    };

    template<typename...>
    struct type_sequence;

    template<typename>
    struct take_type_sequence;

    template<template<typename...> typename T, typename... Types>
    struct take_type_sequence<T<Types...>>
    {
        template<template<typename...> typename U>
        using apply_t = U<Types...>;

        using as_sequence_t = regular_type_sequence<Types...>;

        using as_type_sequence_t = type_sequence<Types...>;
    };

    inline namespace literals
    {
        template<::std::size_t Size>
        struct ltr : ::std::array<char8_t, Size>
        {
            using base = ::std::array<char8_t, Size>;
            using base::base;

            // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays,hicpp-explicit-conversions)
            constexpr ltr(const char8_t (&arr)[Size]) noexcept: ltr::base(::std::to_array(arr)) {}

            // NOLINTNEXTLINE(hicpp-explicit-conversions)
            constexpr ltr(const ::std::array<char8_t, Size>& arr) noexcept: ltr::base(arr) {}

            // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
            constexpr ltr& operator=(const char8_t (&arr)[Size]) noexcept
            {
                *this = ::std::to_array(arr);
            }

            constexpr ltr& operator=(const ::std::array<char8_t, Size>& arr) noexcept
            {
                *this = arr;
            }

            // NOLINTNEXTLINE(hicpp-explicit-conversions)
            constexpr operator ::std::u8string_view() const noexcept
            {
                return {this->data(), Size - 1};
            }

            constexpr auto to_string_view() const noexcept
            {
                return static_cast<::std::u8string_view>(*this); //
            }
        };

        template<ltr ltr>
        constexpr auto operator"" _ltr() noexcept
        {
            return ltr;
        }
    }
}

namespace stdsharp::inline literals
{
    using namespace stdsharp::type_traits::literals;
}
