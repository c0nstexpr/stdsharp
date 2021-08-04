// Created by BlurringShadow at 2021-02-27-下午 10:24

#pragma once

#include <utility>
#include <array>
#include <vector>
#include <string>
#include <string_view>

namespace blurringshadow::utility
{
    using namespace std::literals;

    namespace details
    {
        template<typename T>
        struct auto_cast
        {
            T t;

            template<typename U> // NOLINTNEXTLINE(hicpp-explicit-conversions)
            [[nodiscard]] constexpr operator U() const&& noexcept(noexcept(static_cast<U>(t)))
            {
                return static_cast<U>(t);
            }

            template<typename U>
            [[nodiscard]] constexpr auto operator()() const&& noexcept(noexcept(static_cast<U>(t)))
            {
                return static_cast<U>(t);
            }
        };
    }

    inline constexpr auto auto_cast = []<typename T>(T&& t) //
        noexcept(::std::is_nothrow_constructible_v<details::auto_cast<T>, T>)
    {
        return ::blurringshadow::utility::details::auto_cast<T>{::std::forward<T>(t)}; //
    };

    namespace details
    {
        struct to_underlying_fn
        {
            template<typename T>
                requires ::std::is_enum_v<T>
            [[nodiscard]] constexpr auto operator()(const T v) const noexcept
            {
                return static_cast<::std::underlying_type_t<::std::remove_cvref_t<T>>>(v);
            }
        };
    }

    inline constexpr ::blurringshadow::utility::details::to_underlying_fn to_underlying{};

    template<typename T>
    struct value_wrapper : ::std::type_identity<T>
    {
        T value;

        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr value_wrapper(T&& v) noexcept: value(::std::move(v)) {}
    };

    template<typename T>
    struct value_wrapper<T&> : ::std::type_identity<T&>
    {
        T& value;

        constexpr value_wrapper(T& v) noexcept: value(v) {} // NOLINT(hicpp-explicit-conversions)
    };

    template<typename T>
    using extend_t = ::std::conditional_t<::std::is_rvalue_reference_v<T&&>, T, T&>;

    template<typename T>
    value_wrapper(T&&) -> value_wrapper<extend_t<T>>;
}
