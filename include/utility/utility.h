// Created by BlurringShadow at 2021-02-27-下午 10:24

#pragma once

#include <gsl/gsl>
#include <concepts>

namespace blurringshadow::utility
{
    using namespace std::literals;

    template<typename T>
    struct auto_cast
    {
        T&& t;


        template<typename U>
        // clang-format off
        [[nodiscard]] constexpr operator U() && 
            noexcept(noexcept(static_cast<U>(std::forward<T>(t))))
        // clang-format on
        {
            return static_cast<U>(std::forward<T>(t));
        }
    };

    template<typename T>
    auto_cast(T&& t) -> auto_cast<T>;

    template<typename T>
        requires std::is_enum_v<T>
    [[nodiscard]] constexpr auto to_underlying(const T v)
    {
        return static_cast<std::underlying_type_t<T>>(v);
    }

    template<typename T>
        requires std::convertible_to<T, std::remove_const_t<T>>
    constexpr std::remove_const_t<T> another(T&& t) { return std::forward<T>(t); }
}
