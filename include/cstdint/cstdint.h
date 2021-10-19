#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace stdsharp::cstdint
{
    using i8 = ::std::int8_t; ///< 8-bit signed integer type.
    using u8 = ::std::uint8_t; ///< 8-bit unsigned integer type.
    using i16 = ::std::int16_t; ///< 16-bit signed integer type.
    using u16 = ::std::uint16_t; ///< 16-bit unsigned integer type.
    using i32 = ::std::int32_t; ///< 32-bit signed integer type.
    using u32 = ::std::uint32_t; ///< 32-bit unsigned integer type.
    using i64 = ::std::int64_t; ///< 64-bit signed integer type.
    using u64 = ::std::uint64_t; ///< 64-bit unsigned integer type.
    using ssize_t =
        ::std::make_signed_t<::std::size_t>; ///< Signed integer type corresponding to `size_t`.

    inline namespace literals
    {
        [[nodiscard]] constexpr auto operator""_i8(const unsigned long long value) noexcept
        {
            return static_cast<::stdsharp::cstdint::i8>(value);
        }

        [[nodiscard]] constexpr auto operator""_u8(const unsigned long long value) noexcept
        {
            return static_cast<::stdsharp::cstdint::u8>(value);
        }

        [[nodiscard]] constexpr auto operator""_i16(const unsigned long long value) noexcept
        {
            return static_cast<::stdsharp::cstdint::i16>(value);
        }

        [[nodiscard]] constexpr auto operator""_u16(const unsigned long long value) noexcept
        {
            return static_cast<::stdsharp::cstdint::u16>(value);
        }

        [[nodiscard]] constexpr auto operator""_i32(const unsigned long long value) noexcept
        {
            return static_cast<::stdsharp::cstdint::i32>(value);
        }

        [[nodiscard]] constexpr auto operator""_u32(const unsigned long long value) noexcept
        {
            return static_cast<::stdsharp::cstdint::u32>(value);
        }

        [[nodiscard]] constexpr auto operator""_i64(const unsigned long long value) noexcept
        {
            return static_cast<::stdsharp::cstdint::i64>(value);
        }

        [[nodiscard]] constexpr auto operator""_u64(const unsigned long long value) noexcept
        {
            return static_cast<::stdsharp::cstdint::u64>(value);
        }

        [[nodiscard]] constexpr auto operator""_uz(const unsigned long long value) noexcept
        {
            return static_cast<::std::size_t>(value);
        }

        [[nodiscard]] constexpr auto operator""_z(const unsigned long long value) noexcept
        {
            return static_cast<::stdsharp::cstdint::ssize_t>(value);
        }
    }
}
