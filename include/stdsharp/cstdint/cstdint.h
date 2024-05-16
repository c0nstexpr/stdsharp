#pragma once

#include "../namespace_alias.h"

#include <climits>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#if __has_include(<stdfloat>)

#include <stdfloat>

namespace stdsharp
{
    using f16 = std::float16_t;
}

#endif

namespace stdsharp
{
    inline constexpr std::size_t char_bit = CHAR_BIT;

    using ushort = unsigned short;
    using ulong = unsigned long;
    using ull = unsigned long long;

    using byte = std::underlying_type_t<std::byte>;
    using i8 = std::int8_t;
    using u8 = std::uint8_t;
    using i16 = std::int16_t;
    using u16 = std::uint16_t;
    using i32 = std::int32_t;
    using u32 = std::uint32_t;
    using i64 = std::int64_t;
    using u64 = std::uint64_t;

    using ssize_t = std::make_signed_t<std::size_t>;

    inline constexpr struct
    {
        template<typename T>
        [[nodiscard]] constexpr auto operator()(const T t) noexcept
        {
            return static_cast<std::make_unsigned_t<T>>(t);
        }
    } make_unsigned{};

    inline constexpr struct
    {
        template<typename T>
        [[nodiscard]] constexpr auto operator()(const T t) noexcept
        {
            return static_cast<std::make_signed_t<T>>(t);
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