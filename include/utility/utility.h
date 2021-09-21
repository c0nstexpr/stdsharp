// Created by BlurringShadow at 2021-02-27-下午 10:24

#pragma once

#include <utility>
#include <array>
#include <vector>
#include <string>
#include <string_view>

#include "concepts/concepts.h"

namespace stdsharp::utility
{
    using namespace std::literals;

    namespace details
    {
        template<typename T>
        struct auto_cast
        {
            T&& t;

            template<typename U>
                requires requires
                {
                    static_cast<U>(t);
                } // NOLINTNEXTLINE(hicpp-explicit-conversions)
            [[nodiscard]] constexpr operator U() const&& noexcept(noexcept(static_cast<U>(t)))
            {
                return static_cast<U>(t);
            }

            template<typename U>
            [[nodiscard]] constexpr U operator()() const&& noexcept(noexcept(static_cast<U>(t)))
            {
                return static_cast<U>(*this);
            }
        };
    }

    inline constexpr struct
    {
        template<typename T>
        [[nodiscard]] constexpr auto operator()(T&& t) const noexcept
        {
            return ::stdsharp::utility::details::auto_cast<T>{::std::forward<T>(t)}; //
        }
    } auto_cast{};

    inline constexpr struct
    {
        template<typename T>
            requires ::std::is_enum_v<T>
        [[nodiscard]] constexpr auto operator()(const T v) const noexcept
        {
            return static_cast<::std::underlying_type_t<T>>(v);
        }
    } to_underlying{};
}
