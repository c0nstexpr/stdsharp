#pragma once

#include <filesystem>
#include <limits>
#include <ratio>

#if __cpp_lib_format >= 201907L
    #include <format>
#endif

#include "cstdint/cstdint.h"
#include "default_operator.h"
#include "pattern_match.h"

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
    }

    using bits = space_size<::std::ratio<1, char_bit>>;

    using bytes = space_size<::std::ratio<1>>;

    using kilobytes = space_size<::std::kilo>;

    using megabytes = space_size<::std::mega>;

    using gigabytes = space_size<::std::giga>;

    using terabytes = space_size<::std::tera>;

    using petabytes = space_size<::std::peta>;

    using exabytes = space_size<::std::exa>;

#if(INTMAX_MAX / 1'000'000'000) >= 1'000'000'000'000
    using zettabytes = space_size<::std::zetta>;

    inline namespace literals
    {
        constexpr auto operator""_ZB(unsigned long long v) noexcept { return zettabytes{v}; }
    }

    template<typename CharT, typename Traits>
    auto& operator<<(std::basic_ostream<CharT, Traits>& os, const zettabytes size)
    {
        return os << size.size() << " ZB";
    };

    #if(INTMAX_MAX / 1'000'000'000) >= 1'000'000'000'000'000
    using yottabytes = space_size<::std::yotta>;

    inline namespace literals
    {
        constexpr auto operator""_YB(unsigned long long v) noexcept { return yottabytes{v}; }
    }

    template<typename CharT, typename Traits>
    auto& operator<<(std::basic_ostream<CharT, Traits>& os, const yottabytes size)
    {
        return os << size.size() << " YB";
    };
    #endif
#endif

    using kibibytes = space_size<::std::ratio<details::size_numeration_base>>;

    using mebibytes =
        space_size<::std::ratio<details::size_numeration_base * kibibytes::period::num>>;
    using gibibytes =
        space_size<::std::ratio<details::size_numeration_base * mebibytes::period::num>>;
    using tebibytes =
        space_size<::std::ratio<details::size_numeration_base * gibibytes::period::num>>;
    using pebibytes =
        space_size<::std::ratio<details::size_numeration_base * tebibytes::period::num>>;
    using exbibytes =
        space_size<::std::ratio<details::size_numeration_base * pebibytes::period::num>>;

#if(INTMAX_MAX / 1024) >= 1'152'921'504'606'846'976
    using zebibytes =
        space_size<::std::ratio<details::size_numeration_base * exbibytes::period::num>>;

    inline namespace literals
    {
        constexpr auto operator""_ZiB(unsigned long long v) noexcept { return zebibytes{v}; }
    }

    template<typename CharT, typename Traits>
    auto& operator<<(std::basic_ostream<CharT, Traits>& os, const zebibytes size)
    {
        return os << size.size() << " ZiB";
    };

    #if(INTMAX_MAX / 1024 / 1024) >= 1'152'921'504'606'846'976
    using yobibytes =
        space_size<::std::ratio<details::size_numeration_base * zebibytes::period::num>>;

    inline namespace literals
    {
        constexpr auto operator""_YiB(unsigned long long v) noexcept { return yobibytes{v}; }
    }

    template<typename CharT, typename Traits>
    auto& operator<<(std::basic_ostream<CharT, Traits>& os, const yobibytes size)
    {
        return os << size.size() << " YiB";
    };
    #endif
#endif

    template<typename CharT, typename Traits, typename Period>
    auto& operator<<(std::basic_ostream<CharT, Traits>& os, const space_size<Period> size)
    {
        constexpr_pattern_match::from_type<space_size<Period>>( //
            [&](const ::std::type_identity<bits>) { os << size.size() << "bits"; },
            [&](const ::std::type_identity<bytes>) { os << size.size() << "B"; },
            [&](const ::std::type_identity<kilobytes>) { os << size.size() << "KB"; },
            [&](const ::std::type_identity<megabytes>) { os << size.size() << "MB"; },
            [&](const ::std::type_identity<gigabytes>) { os << size.size() << "GB"; },
            [&](const ::std::type_identity<terabytes>) { os << size.size() << "TB"; },
            [&](const ::std::type_identity<petabytes>) { os << size.size() << "PB"; },
            [&](const ::std::type_identity<exabytes>) { os << size.size() << "EB"; },
            [&](const ::std::type_identity<kibibytes>) { os << size.size() << "KiB"; },
            [&](const ::std::type_identity<mebibytes>) { os << size.size() << "MiB"; },
            [&](const ::std::type_identity<gibibytes>) { os << size.size() << "GiB"; },
            [&](const ::std::type_identity<tebibytes>) { os << size.size() << "TiB"; },
            [&](const ::std::type_identity<pebibytes>) { os << size.size() << "PiB"; },
            [&](const ::std::type_identity<exbibytes>) { os << size.size() << "EiB"; } //
        );

        return os;
    };

    inline namespace literals
    {
        constexpr auto operator""_bit(unsigned long long v) noexcept { return bits{v}; }

        constexpr auto operator""_B(unsigned long long v) noexcept { return bytes{v}; }

        constexpr auto operator""_KB(unsigned long long v) noexcept { return kilobytes{v}; }

        constexpr auto operator""_MB(unsigned long long v) noexcept { return megabytes{v}; }

        constexpr auto operator""_GB(unsigned long long v) noexcept { return gigabytes{v}; }

        constexpr auto operator""_TB(unsigned long long v) noexcept { return terabytes{v}; }

        constexpr auto operator""_PB(unsigned long long v) noexcept { return petabytes{v}; }

        constexpr auto operator""_EB(unsigned long long v) noexcept { return exabytes{v}; }

        constexpr auto operator""_KiB(unsigned long long v) noexcept { return kibibytes{v}; }

        constexpr auto operator""_MiB(unsigned long long v) noexcept { return mebibytes{v}; }

        constexpr auto operator""_GiB(unsigned long long v) noexcept { return gibibytes{v}; }

        constexpr auto operator""_TiB(unsigned long long v) noexcept { return tebibytes{v}; }

        constexpr auto operator""_PiB(unsigned long long v) noexcept { return pebibytes{v}; }

        constexpr auto operator""_EiB(unsigned long long v) noexcept { return exbibytes{v}; }
    }
}

namespace stdsharp::inline literals
{
    using namespace stdsharp::filesystem::literals;
}

#if __cpp_lib_format >= 201907L

namespace std
{
    template<typename Period, typename CharT>
    struct formatter<::stdsharp::filesystem::space_size<Period>, CharT> :
        formatter<::std::uintmax_t, CharT>
    {
        using space_size = ::stdsharp::filesystem::space_size<Period>;
        // parse() is inherited from the base class

        template<typename FormatContext>
        auto format(const space_size s, FormatContext& fc)
        {
            return std::format_to(fc.out(), "{}", s.size());
        }
    };
}
#endif