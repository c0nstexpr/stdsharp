#pragma once

#include "../cstdint/cstdint.h"
#include "../default_operator.h"
#include "../format/format.h"
#include "../functional/sequenced_invocables.h"

#include <cstdint>
#include <format>
#include <ratio>

namespace stdsharp::filesystem
{
    template<typename, typename>
    class space_size;

    template<typename Rep, std::uintmax_t Num, std::uintmax_t Denom>
        requires(std::unsigned_integral<Rep> || std::floating_point<Rep>)
    class space_size<Rep, std::ratio<Num, Denom>> : public default_operator::arithmetic
    {
    public:
        using rep = Rep;

        using period = std::ratio<Num, Denom>::type;

        static constexpr auto num = period::num;

        static constexpr auto denom = period::den;

    private:
        rep size_{};

        template<auto N, auto D>
        static constexpr auto
            cast_from(const auto factor, const std::ratio<N, D> /*unused*/) noexcept
        {
            return static_cast<rep>(factor * denom * N / (num * D)); // NOLINT(*-integer-division)
        }

    public:
        static constexpr space_size zero() noexcept { return {}; }

        static constexpr space_size max() noexcept { return std::numeric_limits<rep>::max(); }

        space_size() = default;

        explicit constexpr space_size(const rep value) noexcept: size_(value) {}

        template<typename OtherRep, typename Period>
            requires(not_same_as<rep, OtherRep> || not_same_as<period, Period>)
        explicit constexpr space_size(const space_size<OtherRep, Period> other) noexcept:
            size_(cast_from(other.size(), typename Period::type{}))
        {
        }

        [[nodiscard]] constexpr auto operator<=>(const space_size& other) const noexcept
        {
            return size() <=> other.size();
        }

        [[nodiscard]] friend constexpr bool
            operator==(const space_size& lhs, const space_size& rhs) noexcept
        {
            return lhs.size() == rhs.size();
        }

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        constexpr auto& operator+=(const space_size& other) noexcept
        {
            size_ += other.size();
            return *this;
        }

        constexpr auto& operator-=(const space_size& other) noexcept
        {
            size_ -= other.size();
            return *this;
        }

        constexpr auto& operator*=(const rep& factor) noexcept
        {
            size_ *= factor;
            return *this;
        }

        constexpr auto& operator/=(const rep& factor) noexcept
        {
            size_ /= factor;
            return *this;
        }
    };
}

namespace stdsharp::filesystem::details
{
    template<typename T, typename>
    inline constexpr std::conditional_t<true, void, T> space_size_unit_name;
}

namespace stdsharp::filesystem
{
    using kibi = std::ratio<1024>; // NOLINT(*-magic-numbers)
    using mebi = std::ratio_multiply<kibi, kibi>;
    using gibi = std::ratio_multiply<mebi, kibi>;
    using tebi = std::ratio_multiply<gibi, kibi>;
    using pebi = std::ratio_multiply<tebi, kibi>;
    using exbi = std::ratio_multiply<pebi, kibi>;

    using bits = space_size<std::uintmax_t, std::ratio<1, char_bit>>;
    using bytes = space_size<std::uintmax_t, std::ratio<1>>;
    using kilobytes = space_size<std::uintmax_t, std::kilo>;
    using megabytes = space_size<std::uintmax_t, std::mega>;
    using gigabytes = space_size<std::uintmax_t, std::giga>;
    using terabytes = space_size<std::uintmax_t, std::tera>;
    using petabytes = space_size<std::uintmax_t, std::peta>;
    using exabytes = space_size<std::uintmax_t, std::exa>;

#define STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(period, unit_name)                             \
    template<>                                                                                 \
    inline constexpr std::string_view details::space_size_unit_name<period, char>{#unit_name}; \
    template<>                                                                                 \
    inline constexpr std::wstring_view details::                                               \
        space_size_unit_name<period, wchar_t>{L#unit_name};                                    \
                                                                                               \
    inline namespace literals                                                                  \
    {                                                                                          \
        [[nodiscard]] constexpr auto operator""_##unit_name(unsigned long long v) noexcept     \
        {                                                                                      \
            return space_size<unsigned long long, period>{v};                                  \
        }                                                                                      \
                                                                                               \
        [[nodiscard]] constexpr auto operator""_##unit_name(long double v) noexcept            \
        {                                                                                      \
            return space_size<long double, period>{v};                                         \
        }                                                                                      \
    }                                                                                          \
                                                                                               \
    template<typename CharT, typename Traits, typename Rep>                                    \
    constexpr auto& operator<<(                                                                \
        std::basic_ostream<CharT, Traits>& os,                                                 \
        const space_size<Rep, period> size                                                     \
    )                                                                                          \
    {                                                                                          \
        return os << size.size() << #unit_name;                                                \
    }

#if(INTMAX_MAX / 1'000'000'000) >= 1'000'000'000'000
    using zettabytes = space_size<std::uintmax_t, std::zetta>;
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(zetta, ZB)

    #if(INTMAX_MAX / 1'000'000'000) >= 1'000'000'000'000'000
    using yottabytes = space_size<std::uintmax_t, std::yotta>;
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(yotta, YB)

    #endif
#endif

    using kibibytes = space_size<std::uintmax_t, kibi>;
    using mebibytes = space_size<std::uintmax_t, mebi>;
    using gibibytes = space_size<std::uintmax_t, gibi>;
    using tebibytes = space_size<std::uintmax_t, tebi>;
    using pebibytes = space_size<std::uintmax_t, pebi>;
    using exbibytes = space_size<std::uintmax_t, exbi>;

#if(UINTMAX_MAX / 1024) >= 1'152'921'504'606'846'976
    using zebi = std::ratio_multiply<exbi, kibi>;
    using zebibytes = space_size<std::uintmax_t, zebi>;
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(zebi, ZiB)

    #if(INTMAX_MAX / 1024 / 1024) >= 1'152'921'504'606'846'976.
    using yobi = std::ratio_multiply<zebi, kibi>;
    using yobibytes = space_size<std::uintmax_t, yobi>;
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(yobi, YiB)

    #endif
#endif

    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(bits::period, bits);
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(bytes::period, B);
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(kilobytes::period, KB);
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(megabytes::period, MB);
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(gigabytes::period, GB);
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(terabytes::period, TB);
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(petabytes::period, PB);
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(exabytes::period, EB);

    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(kibibytes::period, KiB);
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(mebibytes::period, MiB);
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(gibibytes::period, GiB);
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(tebibytes::period, TiB);
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(pebibytes::period, PiB);
    STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR(exbibytes::period, EiB);

#undef STDSHARP_FILESYSTEM_SPACE_SIZE_OPERATOR
}

namespace stdsharp::inline literals
{
    using namespace stdsharp::filesystem::literals;
}

namespace std
{
#if __cpp_lib_print >= 202403L
    template<typename Rep, typename Period>
    inline constexpr bool
        enable_nonlocking_formatter_optimization<::stdsharp::filesystem::space_size<Rep, Period>> =
            true;

#endif

    template<typename Rep, typename Period, typename CharT>
        requires(same_as<CharT, char> || same_as<CharT, wchar_t>)
    class formatter<::stdsharp::filesystem::space_size<Rep, Period>, CharT>
    {
        using space_size = ::stdsharp::filesystem::space_size<Rep, Period>;

        static constexpr auto default_unit_name = []
        {
            if constexpr(requires {
                             ::stdsharp::filesystem::details::space_size_unit_name<Period, CharT>;
                         })
                return ::stdsharp::filesystem::details::space_size_unit_name<Period, CharT>;
            else return std::basic_string_view<CharT>{};
        }();

        ::stdsharp::fill_and_align_spec<CharT> fill_and_align_;
        ::stdsharp::uint_nested_maybe_spec<> width_;
        ::stdsharp::uint_nested_maybe_spec<> precision_;
        bool use_locale_ = false;
        optional<::stdsharp::nested_fmt_spec> unit_name_;

        constexpr decltype(auto) get_format_string() const
        {
            if constexpr(same_as<CharT, char>) return use_locale_ ? "{:.{}L}{}" : "{:.{}}{}";
            else if constexpr(same_as<CharT, wchar_t>)
                return use_locale_ ? L"{:.{}L}{}" : L"{:.{}}{}";
            else throw format_error{"Unsupported character type"};
        }

        constexpr auto get_nested(const auto& ctx, const size_t id)
        {
            return ::stdsharp::visit_fmt_arg<>(
                id,
                ctx,
                ::stdsharp::sequenced_invocables{
                    [](const integral auto value)
                    { return value > 0 ? value : throw format_error{"invalid num"}; },
                    [](const unsigned_integral auto value) { return value; },
                    [] [[noreturn]] (const auto&) { throw format_error{"invalid num"}; }
                }
            );
        }

    public:
        constexpr auto parse(basic_format_parse_context<CharT>& ctx)
        {
            fill_and_align_ = ::stdsharp::parse_fill_and_align_spec(ctx);
            width_ = ::stdsharp::parse_uint_maybe_nested_spec(ctx);

            if(fill_and_align_.align != ::stdsharp::format_align_t::none && width_.index() == 0)
                throw format_error{"Specify align without width"};

            precision_ = ::stdsharp::parse_precision_spec(ctx);
            use_locale_ = ::stdsharp::parse_locale_spec(ctx);
            unit_name_ = ::stdsharp::parse_nested_spec(ctx);
        }

        template<typename OutIt>
        constexpr auto format(const space_size& size, basic_format_context<OutIt, CharT>& ctx) const
        {
            const auto& value = std::format(
                std::basic_format_string<CharT>{get_format_string()},
                size.size(),
                *precision_,
                default_unit_name
            );

            const auto& out = ctx.out();

            if(width_.index() == 0) return ranges::copy(value, out).out;

            const auto width = width_.index() == 1 ? //
                get<1>(width_) :
                get_nested(ctx, get<2>(width_).id);

            if(value.size() >= width) return ranges::copy(value, out).out;
        }
    };
}