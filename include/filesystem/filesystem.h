#pragma once

#include <filesystem>
#include <limits>
#include <ratio>

#include "cstdint/cstdint.h"
#include "default_operator.h"

namespace stdsharp::filesystem
{
    template<typename Period>
    class space_size;

    namespace details
    {
        struct space_size_delegate
        {
            template<typename Period>
            constexpr auto& operator()(space_size<Period>& t) const noexcept
            {
                return t.value_;
            }

            template<typename Period>
            constexpr auto operator()(const space_size<Period>& t) const noexcept
            {
                return t.value_;
            }
        };
    }

    template<auto Num, auto Denom>
    class [[nodiscard]] space_size<::std::ratio<Num, Denom>> :
        default_arithmetic_assign_operation<
            space_size<::std::ratio<Num, Denom>>,
            details::space_size_delegate // clang-format off
        >, // clang-format on
        default_arithmetic_operation<
            space_size<::std::ratio<Num, Denom>>,
            true,
            details::space_size_delegate // clang-format off
        > // clang-format on
    {
        friend struct details::space_size_delegate;

        template<typename>
        friend class space_size;

        using value_type = ::std::uintmax_t;

        value_type value_{};

        template<auto N, auto D>
        static constexpr auto from_ratio(const auto factor, const ::std::ratio<N, D>) noexcept
        {
            return factor * N / D;
        }

    public:
        using period = ::std::ratio<Num, Denom>;

        static constexpr space_size zero() noexcept { return {}; }
        static constexpr space_size max() noexcept
        {
            return ::std::numeric_limits<value_type>::max();
        }

        space_size() = default;

        explicit constexpr space_size(const value_type value) noexcept: value_(value) {}

        template<typename Period>
        constexpr space_size(const space_size<Period> other) noexcept:
            value_(from_ratio(other.value_, ::std::ratio_divide<Period, period>{}))
        {
        }

        template<typename Period>
        [[nodiscard]] constexpr auto operator<=>(const space_size<Period> other) const noexcept
        {
            if constexpr(::std::ratio_greater_v<period, Period>)
                return value_ <=> from_ratio(other.value_, ::std::ratio_divide<Period, period>{});
            else
                return other.value_ <=> from_ratio(value_, ::std::ratio_divide<period, Period>{});
        }

        template<typename Period>
        [[nodiscard]] constexpr auto operator==(const space_size<Period> other) const noexcept
        {
            return (*this <=> other) == ::std::strong_ordering::equal;
        }

        template<typename Period>
        [[nodiscard]] constexpr auto operator!=(const space_size<Period> other) const noexcept
        {
            return !(*this == other);
        }

        [[nodiscard]] constexpr auto size() const noexcept { return value_; }
    };

    namespace details
    {
        inline constexpr ::std::uintmax_t size_numeration_base = 1024;
        inline constexpr ::std::uintmax_t size_dec_numeration_base = 1000;
    }

    using bits = space_size<::std::ratio<1, char_bit>>;

    constexpr auto operator""_bit(unsigned long long v) noexcept { return bits{v}; }

    using bytes = space_size<::std::ratio<1>>;

    constexpr auto operator""_byte(unsigned long long v) noexcept { return bytes{v}; }

    using kilobytes = space_size<::std::ratio<1, details::size_dec_numeration_base>>;

    constexpr auto operator""_KB(unsigned long long v) noexcept { return kilobytes{v}; }

    using megabytes =
        space_size<::std::ratio<details::size_dec_numeration_base * kilobytes::period::den>>;

    constexpr auto operator""_MB(unsigned long long v) noexcept { return megabytes{v}; }

    using gigabytes =
        space_size<::std::ratio<details::size_dec_numeration_base * megabytes::period::den>>;

    constexpr auto operator""_GB(unsigned long long v) noexcept { return gigabytes{v}; }

    using terabytes =
        space_size<::std::ratio<details::size_dec_numeration_base * gigabytes::period::den>>;

    constexpr auto operator""_TB(unsigned long long v) noexcept { return terabytes{v}; }

    using petabytes =
        space_size<::std::ratio<details::size_dec_numeration_base * terabytes::period::den>>;

    constexpr auto operator""_PB(unsigned long long v) noexcept { return petabytes{v}; }

    using exabytes =
        space_size<::std::ratio<details::size_dec_numeration_base * petabytes::period::den>>;

    constexpr auto operator""_EB(unsigned long long v) noexcept { return exabytes{v}; }

    using zettabytes =
        space_size<::std::ratio<details::size_dec_numeration_base * exabytes::period::den>>;

    constexpr auto operator""_ZB(unsigned long long v) noexcept { return zettabytes{v}; }

    using yottabytes =
        space_size<::std::ratio<details::size_dec_numeration_base * zettabytes::period::den>>;

    constexpr auto operator""_YB(unsigned long long v) noexcept { return yottabytes{v}; }


    using kibibytes = space_size<::std::ratio<1, details::size_numeration_base>>;

    constexpr auto operator""_KiB(unsigned long long v) noexcept { return kibibytes{v}; }

    using mebibytes =
        space_size<::std::ratio<details::size_numeration_base * kilobytes::period::den>>;

    constexpr auto operator""_MiB(unsigned long long v) noexcept { return mebibytes{v}; }
    using gibibytes =
        space_size<::std::ratio<details::size_numeration_base * megabytes::period::den>>;

    constexpr auto operator""_GiB(unsigned long long v) noexcept { return gibibytes{v}; }
    using tebibytes =
        space_size<::std::ratio<details::size_numeration_base * gigabytes::period::den>>;

    constexpr auto operator""_TiB(unsigned long long v) noexcept { return tebibytes{v}; }
    using pebibytes =
        space_size<::std::ratio<details::size_numeration_base * terabytes::period::den>>;

    constexpr auto operator""_PiB(unsigned long long v) noexcept { return pebibytes{v}; }
    using exbibytes =
        space_size<::std::ratio<details::size_numeration_base * petabytes::period::den>>;

    constexpr auto operator""_EiB(unsigned long long v) noexcept { return exbibytes{v}; }
    using zebibytes =
        space_size<::std::ratio<details::size_numeration_base * exabytes::period::den>>;

    constexpr auto operator""_ZiB(unsigned long long v) noexcept { return zebibytes{v}; }
    using yobibytes =
        space_size<::std::ratio<details::size_numeration_base * zettabytes::period::den>>;

    constexpr auto operator""_YiB(unsigned long long v) noexcept { return yobibytes{v}; }
}