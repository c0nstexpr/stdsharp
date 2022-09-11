#pragma once

#include "utility/utility.h"
#include "concepts/concepts.h"

namespace stdsharp
{
    template<concepts::enumeration T>
    struct enumeration
    {
        using enum_type = T;
        using underlying_type = ::std::underlying_type_t<T>;

        T value{};

        explicit constexpr operator underlying_type() const noexcept { return auto_cast(value); }

        explicit constexpr operator T&() noexcept { return value; }

        explicit constexpr operator T() const noexcept { return value; }

        constexpr auto operator==(const T other) const noexcept { return value == other; }

        constexpr auto operator!=(const T other) const noexcept { return !(*this == other); }

        friend constexpr auto operator==(const T v, const enumeration e) noexcept { return e == v; }

        friend constexpr auto operator!=(const T v, const enumeration e) noexcept { return e != v; }
    };

    template<typename T>
    enumeration(T) -> enumeration<::std::remove_cv_t<T>>;

    template<concepts::enumeration T>
    struct flag : enumeration<T>
    {
        using base = enumeration<T>;
        using base::base;
        using base::value;

        template<typename U>
        constexpr flag(const U t) noexcept: base{t}
        {
        }

        // NOLINTBEGIN(hicpp-signed-bitwise)
        constexpr auto operator|(const T other) const noexcept { return flag{value | other}; }

        friend constexpr auto operator|(const T v, const flag e) noexcept { return flag{e | v}; }

        constexpr flag operator|(const flag other) const noexcept { return flag{value | other}; }

        constexpr flag operator&(const T other) const noexcept { return flag{value & other}; }

        friend constexpr auto operator&(const T v, const flag e) noexcept { return flag{e & v}; }

        constexpr auto operator&(const flag other) const noexcept { return flag{value & other}; }

        // NOLINTEND(hicpp-signed-bitwise)
    };

    template<typename T>
    flag(T) -> flag<::std::remove_cv_t<T>>;
}