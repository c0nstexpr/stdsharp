#pragma once

#include <climits>
#include <limits>
#include <stdexcept>

#include "../utility/auto_cast.h"

namespace stdsharp
{
    inline constexpr std::size_t char_bit = CHAR_BIT;

    using byte = std::underlying_type_t<std::byte>;
    using i8 = std::int8_t; ///< 8-bit signed integer type.
    using u8 = std::uint8_t; ///< 8-bit unsigned integer type.
    using i16 = std::int16_t; ///< 16-bit signed integer type.
    using u16 = std::uint16_t; ///< 16-bit unsigned integer type.
    using i32 = std::int32_t; ///< 32-bit signed integer type.
    using u32 = std::uint32_t; ///< 32-bit unsigned integer type.
    using i64 = std::int64_t; ///< 64-bit signed integer type.
    using u64 = std::uint64_t; ///< 64-bit unsigned integer type.
    using ssize_t =
        std::make_signed_t<std::size_t>; ///< Signed integer type corresponding to `size_t`.

    inline constexpr struct
    {
        template<typename T>
        [[nodiscard]] constexpr std::make_unsigned_t<T> operator()(const T t) noexcept
        {
            return auto_cast(t);
        }
    } make_unsigned{};

    inline constexpr struct
    {
        template<typename T>
        [[nodiscard]] constexpr std::make_signed_t<T> operator()(const T t) noexcept
        {
            return auto_cast(t);
        }
    } make_signed{};

    inline namespace literals
    {
#define STDSHARP_INT_LITERALS(literal)                                                         \
    [[nodiscard]] constexpr auto operator""_##literal(const unsigned long long value) noexcept \
    {                                                                                          \
        return static_cast<literal>(value);                                                    \
    }

#define STDSHARP_SIGNS_INT_LITERALS(num) STDSHARP_INT_LITERALS(i##num) STDSHARP_INT_LITERALS(u##num)

        STDSHARP_SIGNS_INT_LITERALS(8)
        STDSHARP_SIGNS_INT_LITERALS(16)
        STDSHARP_SIGNS_INT_LITERALS(32)
        STDSHARP_SIGNS_INT_LITERALS(64)

#undef STDSHARP_SIGNS_INT_LITERALS

        STDSHARP_INT_LITERALS(byte)

#undef STDSHARP_INT_LITERALS
    }
}