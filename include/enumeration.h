#pragma once

#include "utility/utility.h"
#include "concepts/concepts.h"

namespace stdsharp
{
    template<concepts::enumeration T>
    struct enumeration
    {
        using underlying_type = ::std::underlying_type_t<T>;

        T value{};

        explicit constexpr enumeration(const underlying_type t) noexcept: value(auto_cast(t)) {}

        explicit constexpr operator underlying_type() const noexcept { return auto_cast(value); }
        explicit constexpr operator T&() noexcept { return value; }
        explicit constexpr operator const T&() const noexcept { return value; }

        constexpr auto operator==(const T other) const noexcept { return value == other; }
        constexpr auto operator!=(const T other) const noexcept { return !(*this == other); }

        friend constexpr auto operator==(const T v, const enumeration e) noexcept { return e == v; }
        friend constexpr auto operator!=(const T v, const enumeration e) noexcept { return e != v; }
    };

    template<typename T>
    enumeration(T) -> enumeration<T>;

    template<concepts::enumeration T>
    struct flag : enumeration<T>
    {
        using base = enumeration<T>;
        using base::base;

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        constexpr auto operator|(const T other) const noexcept { return flag{value | other}; }

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        friend constexpr auto operator|(const T v, const flag e) noexcept { return flag{e | v}; }

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        constexpr flag operator|(const flag other) const noexcept { return flag{value | other}; }

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        constexpr flag operator&(const T other) const noexcept { return flag{value & other}; }

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        friend constexpr auto operator&(const T v, const flag e) noexcept { return flag{e & v}; }

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        constexpr auto operator&(const flag other) const noexcept { return flag{value & other}; }
    };

    template<typename T>
    flag(T) -> flag<T>;
}