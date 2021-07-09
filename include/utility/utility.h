// Created by BlurringShadow at 2021-02-27-下午 10:24

#pragma once

#include <gsl/gsl>
#include <utility>
#include <concepts>

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
        noexcept(std::is_nothrow_constructible_v<details::auto_cast<T>, T>)
    {
        return details::auto_cast<T>{std::forward<T>(t)}; //
    };

    namespace details
    {
        using namespace std;

        struct to_underlying_fn
        {
            template<typename T>
                requires is_enum_v<T>
            [[nodiscard]] constexpr underlying_type_t<T> operator()(const T v) const noexcept
            {
                return v;
            }
        };
    }

    inline constexpr details::to_underlying_fn to_underlying{};
}
