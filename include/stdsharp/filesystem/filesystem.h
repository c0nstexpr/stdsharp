#pragma once

#include <filesystem>
#include <ratio>

#include "../format/format.h"
#include "../default_operator.h"
#include "../pattern_match.h"

namespace stdsharp::filesystem
{
    template<typename, typename>
    class space_size;

    namespace details
    {
        struct space_size_delegate
        {
            template<typename Rep, typename Period>
            constexpr auto& operator()(space_size<Rep, Period>& t) const noexcept
            {
                return t.value_;
            }

            template<typename Rep, typename Period>
            constexpr auto operator()(const space_size<Rep, Period>& t) const noexcept
            {
                return t.value_;
            }
        };
    }

    template<typename Rep, ::std::uintmax_t Num, ::std::uintmax_t Denom>
        requires(!::std::same_as<::std::ratio<Num, Denom>, typename ::std::ratio<Num, Denom>::type>)
    class space_size<Rep, ::std::ratio<Num, Denom>> :
        public space_size<Rep, typename ::std::ratio<Num, Denom>::type>
    {
    };

    template<typename Rep, ::std::uintmax_t Num, ::std::uintmax_t Denom>
    class space_size<Rep, ::std::ratio<Num, Denom>> :
        default_arithmetic_assign_operation<
            space_size<Rep, ::std::ratio<Num, Denom>>,
            details::space_size_delegate // clang-format off
        >, // clang-format on
        default_arithmetic_operation<
            space_size<Rep, ::std::ratio<Num, Denom>>,
            true,
            details::space_size_delegate // clang-format off
        > // clang-format on
    {
    public:
        using rep = Rep;

    private:
        friend struct details::space_size_delegate;

        template<typename, typename>
        friend class space_size;

        rep value_{};

        template<auto N, auto D>
        static constexpr auto cast_from(const auto factor, const ::std::ratio<N, D>) noexcept
        {
            return static_cast<rep>(factor * Denom * N / (Num * D)); // NOLINT(*-integer-division)
        }

    public:
        using period = ::std::ratio<Num, Denom>;

        static constexpr space_size zero() noexcept { return {}; }

        static constexpr space_size max() noexcept { return ::std::numeric_limits<rep>::max(); }

        space_size() = default;

        explicit constexpr space_size(const rep value) noexcept: value_(value) {}

        template<typename OtherRep, typename Period>
        constexpr space_size(const space_size<OtherRep, Period> other) noexcept:
            value_(cast_from(other.value_, Period{}))
        {
        }

        template<typename OtherRep, typename Period>
        [[nodiscard]] constexpr auto operator<=>(const space_size<OtherRep, Period> other
        ) const noexcept
        {
            if constexpr(::std::ratio_greater_equal_v<period, Period>)
                return value_ <=> cast_from(other.value_, Period{});
            else return other <=> *this;
        }

        template<typename OtherRep, typename Period>
        [[nodiscard]] constexpr auto operator==(const space_size<OtherRep, Period> other
        ) const noexcept
        {
            return (*this <=> other) == ::std::strong_ordering::equal;
        }

        template<typename OtherRep, typename Period>
        [[nodiscard]] constexpr auto operator!=(const space_size<OtherRep, Period> other
        ) const noexcept
        {
            return !(*this == other);
        }

        [[nodiscard]] constexpr auto size() const noexcept { return value_; }
    };

    namespace details
    {
        inline constexpr ::std::uintmax_t size_numeration_base = 1024;
    }

    using bits = space_size<::std::uintmax_t, ::std::ratio<1, char_bit>>;

    using bytes = space_size<::std::uintmax_t, ::std::ratio<1>>;

    using kilobytes = space_size<::std::uintmax_t, ::std::kilo>;

    using megabytes = space_size<::std::uintmax_t, ::std::mega>;

    using gigabytes = space_size<::std::uintmax_t, ::std::giga>;

    using terabytes = space_size<::std::uintmax_t, ::std::tera>;

    using petabytes = space_size<::std::uintmax_t, ::std::peta>;

    using exabytes = space_size<::std::uintmax_t, ::std::exa>;

#if(INTMAX_MAX / 1'000'000'000) >= 1'000'000'000'000
    using zettabytes = space_size<::std::uintmax_t, ::std::zetta>;

    inline namespace literals
    {
        [[nodiscard]] constexpr auto operator""_ZB(unsigned long long v) noexcept
        {
            return zettabytes{v};
        }

        [[nodiscard]] constexpr auto operator""_ZB(long double v) noexcept
        {
            return space_size<long double, ::std::zetta>{v};
        }
    }

    template<typename CharT, typename Traits, typename Rep>
    auto& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const space_size<Rep, zettabytes::period> size
    )
    {
        return os << size.size() << " ZB";
    };

    #if(INTMAX_MAX / 1'000'000'000) >= 1'000'000'000'000'000
    using yottabytes = space_size<::std::uintmax_t, ::std::yotta>;

    inline namespace literals
    {
        [[nodiscard]] constexpr auto operator""_YB(unsigned long long v) noexcept
        {
            return yottabytes{v};
        }

        [[nodiscard]] constexpr auto operator""_YB(long double v) noexcept
        {
            return space_size<long double, ::std::yotta>{v};
        }
    }

    template<typename CharT, typename Traits, typename Rep>
    auto& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const space_size<Rep, yottabytes::period> size
    )
    {
        return os << size.size() << " YB";
    };
    #endif
#endif

    using kibibytes = space_size<::std::uintmax_t, ::std::ratio<details::size_numeration_base>>;

    using mebibytes = space_size<
        ::std::uintmax_t,
        ::std::ratio<details::size_numeration_base * kibibytes::period::num>>;
    using gibibytes = space_size<
        ::std::uintmax_t,
        ::std::ratio<details::size_numeration_base * mebibytes::period::num>>;
    using tebibytes = space_size<
        ::std::uintmax_t,
        ::std::ratio<details::size_numeration_base * gibibytes::period::num>>;
    using pebibytes = space_size<
        ::std::uintmax_t,
        ::std::ratio<details::size_numeration_base * tebibytes::period::num>>;
    using exbibytes = space_size<
        ::std::uintmax_t,
        ::std::ratio<details::size_numeration_base * pebibytes::period::num>>;

#if(INTMAX_MAX / 1024) >= 1'152'921'504'606'846'976
    using zebibytes = space_size<
        ::std::uintmax_t,
        ::std::ratio<details::size_numeration_base * exbibytes::period::num>>;

    inline namespace literals
    {
        [[nodiscard]] constexpr auto operator""_ZiB(unsigned long long v) noexcept
        {
            return zebibytes{v};
        }

        [[nodiscard]] constexpr auto operator""_ZiB(long double v) noexcept
        {
            return space_size<long double, zebibytes::period>{v};
        }
    }

    template<typename CharT, typename Traits, typename Rep>
    auto& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const space_size<Rep, zebibytes::period> size
    )
    {
        return os << size.size() << " ZiB";
    };

    #if(INTMAX_MAX / 1024 / 1024) >= 1'152'921'504'606'846'976
    using yobibytes = space_size<
        ::std::uintmax_t,
        ::std::ratio<details::size_numeration_base * zebibytes::period::num>>;

    inline namespace literals
    {
        [[nodiscard]] constexpr auto operator""_YiB(unsigned long long v) noexcept
        {
            return yobibytes{v};
        }

        [[nodiscard]] constexpr auto operator""_YiB(long double v) noexcept
        {
            return space_size<long double, yobibytes::period>{v};
        }
    }

    template<typename CharT, typename Traits, typename Rep>
    auto& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const space_size<Rep, yobibytes::period> size
    )
    {
        return os << size.size() << " YiB";
    };
    #endif
#endif

    template<typename CharT, typename Traits, typename Rep, typename Period>
        requires requires(std::basic_ostream<CharT, Traits> os, Rep rep) //
    {
        requires same_as_any<
            Period,
            bits::period,
            bytes::period,
            kilobytes::period,
            megabytes::period,
            gigabytes::period,
            terabytes::period,
            petabytes::period,
            exabytes::period,
            kibibytes::period,
            mebibytes::period,
            gibibytes::period,
            tebibytes::period,
            pebibytes::period,
            exbibytes::period // clang-format off
        >; // clang-format on
        os << rep;
    }
    auto& operator<<(std::basic_ostream<CharT, Traits>& os, const space_size<Rep, Period> size)
    {
        constexpr auto sv = []
        {
            ::std::string_view sv;
            constexpr_pattern_match::from_type<Period>(
                [&](const ::std::type_identity<bits::period>) { sv = "b"; },
                [&](const ::std::type_identity<bytes::period>) { sv = "B"; },
                [&](const ::std::type_identity<kilobytes::period>) { sv = "KB"; },
                [&](const ::std::type_identity<megabytes::period>) { sv = "MB"; },
                [&](const ::std::type_identity<gigabytes::period>) { sv = "GB"; },
                [&](const ::std::type_identity<terabytes::period>) { sv = "TB"; },
                [&](const ::std::type_identity<petabytes::period>) { sv = "PB"; },
                [&](const ::std::type_identity<exabytes::period>) { sv = "EB"; },
                [&](const ::std::type_identity<kibibytes::period>) { sv = "KiB"; },
                [&](const ::std::type_identity<mebibytes::period>) { sv = "MiB"; },
                [&](const ::std::type_identity<gibibytes::period>) { sv = "GiB"; },
                [&](const ::std::type_identity<tebibytes::period>) { sv = "TiB"; },
                [&](const ::std::type_identity<pebibytes::period>) { sv = "PiB"; },
                [&](const ::std::type_identity<exbibytes::period>) { sv = "EiB"; }
            );

            return sv;
        }();

        return os << size.size() << sv;
    };

    inline namespace literals
    {
        [[nodiscard]] constexpr auto operator""_bit(unsigned long long v) noexcept
        {
            return bits{v};
        }

        [[nodiscard]] constexpr auto operator""_B(unsigned long long v) noexcept
        {
            return bytes{v};
        }

        [[nodiscard]] constexpr auto operator""_KB(unsigned long long v) noexcept
        {
            return kilobytes{v};
        }

        [[nodiscard]] constexpr auto operator""_MB(unsigned long long v) noexcept
        {
            return megabytes{v};
        }

        [[nodiscard]] constexpr auto operator""_GB(unsigned long long v) noexcept
        {
            return gigabytes{v};
        }

        [[nodiscard]] constexpr auto operator""_TB(unsigned long long v) noexcept
        {
            return terabytes{v};
        }

        [[nodiscard]] constexpr auto operator""_PB(unsigned long long v) noexcept
        {
            return petabytes{v};
        }

        [[nodiscard]] constexpr auto operator""_EB(unsigned long long v) noexcept
        {
            return exabytes{v};
        }

        [[nodiscard]] constexpr auto operator""_KiB(unsigned long long v) noexcept
        {
            return kibibytes{v};
        }

        [[nodiscard]] constexpr auto operator""_MiB(unsigned long long v) noexcept
        {
            return mebibytes{v};
        }

        [[nodiscard]] constexpr auto operator""_GiB(unsigned long long v) noexcept
        {
            return gibibytes{v};
        }

        [[nodiscard]] constexpr auto operator""_TiB(unsigned long long v) noexcept
        {
            return tebibytes{v};
        }

        [[nodiscard]] constexpr auto operator""_PiB(unsigned long long v) noexcept
        {
            return pebibytes{v};
        }

        [[nodiscard]] constexpr auto operator""_EiB(unsigned long long v) noexcept
        {
            return exbibytes{v};
        }

        [[nodiscard]] constexpr auto operator""_bit(long double v) noexcept
        {
            return space_size<long double, bits::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_B(long double v) noexcept
        {
            return space_size<long double, bytes::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_KB(long double v) noexcept
        {
            return space_size<long double, kilobytes::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_MB(long double v) noexcept
        {
            return space_size<long double, megabytes::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_GB(long double v) noexcept
        {
            return space_size<long double, gigabytes::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_TB(long double v) noexcept
        {
            return space_size<long double, terabytes::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_PB(long double v) noexcept
        {
            return space_size<long double, petabytes::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_EB(long double v) noexcept
        {
            return space_size<long double, exabytes::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_KiB(long double v) noexcept
        {
            return space_size<long double, kibibytes::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_MiB(long double v) noexcept
        {
            return space_size<long double, mebibytes::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_GiB(long double v) noexcept
        {
            return space_size<long double, gibibytes::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_TiB(long double v) noexcept
        {
            return space_size<long double, tebibytes::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_PiB(long double v) noexcept
        {
            return space_size<long double, pebibytes::period>{v};
        }

        [[nodiscard]] constexpr auto operator""_EiB(long double v) noexcept
        {
            return space_size<long double, exbibytes::period>{v};
        }
    }
}

namespace stdsharp::inline literals
{
    using namespace stdsharp::filesystem::literals;
}

#if __cpp_lib_format >= 201907L
namespace std
#else
namespace fmt
#endif
{
    template<typename Rep, typename Period, typename CharT>
        requires requires(
            ::stdsharp::filesystem::space_size<Rep, Period> s,
            ::std::basic_stringstream<CharT> ss
        ) { ss << s; }
    struct formatter<::stdsharp::filesystem::space_size<Rep, Period>, CharT>
    {
    private:
        using space_size = ::stdsharp::filesystem::space_size<Rep, Period>;

#define FMTSHARP ::stdsharp::fmt
        FMTSHARP::fill_spec<CharT> fill_{};
        FMTSHARP::align_t align_{};
        FMTSHARP::nested_spec<::std::size_t> width_{};

        FMTSHARP::precision_spec precision_{};

        FMTSHARP::locale_spec locale_{};
#undef FMTSHARP

        ::std::basic_string_view<CharT> from_unit_;

        template<typename T>
        using identity = ::std::type_identity<T>;

    public:
        constexpr auto parse(basic_format_parse_context<CharT>& ctx)
        {
            namespace fmtsharp = ::stdsharp::fmt;

            {
                basic_format_parse_context<CharT> copied_ctx{
                    ::std::basic_string_view{ctx.begin(), ctx.end()} //
                };

                const auto size = ::std::ranges::size(copied_ctx);

                const auto fill = fmtsharp::parse_fill_spec(copied_ctx);

                if(fill.fill)
                {
                    const auto align = fmtsharp::parse_align_spec(copied_ctx);

                    if(align != fmtsharp::align_t::none)
                    {
                        const auto width =
                            fmtsharp::parse_nested_integer_spec<::std::size_t>(copied_ctx);

                        if(!width.valueless_by_exception())
                        {
                            fill_ = fill;
                            align_ = align;
                            width_ = width;

                            ctx.advance_to(ctx.begin() + (size - ::std::ranges::size(copied_ctx)));
                        }
                    }
                }
            }

            precision_ = fmtsharp::parse_precision_spec(ctx);
            locale_ = fmtsharp::parse_locale_spec(ctx);

            {
                const auto [from_unit] = ::ctre::starts_with<R"((?:[KMGTPE]i?)?B|b)">(ctx);

                if(from_unit)
                {
                    from_unit_ = from_unit;
                    ctx.advance_to(from_unit.end());
                }
            }

            fmtsharp::parse_end_assert(ctx);

            return ctx.begin();
        }

        template<typename OutputIt>
        auto format(const space_size s, basic_format_context<OutputIt, CharT>& fc) const
        {
            namespace fmtsharp = ::stdsharp::fmt;

            const auto& fill = fill_.fill;
            const auto width = fmtsharp::get_arg(fc, width_);
            const auto& formatted = [this, s = s, &fc]() mutable
            {
                const auto& precision = precision_.precision;
                const auto from_unit = [from_unit_ = from_unit_]
                {
                    ::std::array<char, 4> from_unit{};

                    ::std::ranges::transform(
                        from_unit_,
                        from_unit.begin(),
                        [](const CharT c) { return static_cast<char>(c); }
                    );
                    return from_unit;
                }();

                ::std::string_view current_unit{
                    from_unit.begin(),
                    ::std::ranges::find(from_unit, char{}) //
                };
                ::std::basic_ostringstream<CharT> ss;

                const auto do_format = [&current_unit, &ss, &s]
                {
                    const auto format_case = [&]<typename SpaceSize>( // clang-format off
                        const identity<SpaceSize>,
                        const ::std::string_view next_unit
                    ) noexcept // clang-format on
                    {
                        return [&, next_unit](const ::std::string_view)
                        {
                            const SpaceSize cast_size = s;

                            ss << cast_size;

                            s -= space_size{cast_size};
                            current_unit = next_unit;
                        };
                    };

                    ::stdsharp::pattern_match(
                        current_unit,
                        ::std::pair{
                            [](const ::std::string_view unit) noexcept { return unit.empty(); },
                            [](const ::std::string_view)
                            {
                                throw format_error{"Precision exceeded"}; //
                            } //
                        },
#define STDSHARP_MAKE_PAIR(str, type, next_str)                              \
    ::std::pair{                                                             \
        [](const ::std::string_view unit) noexcept { return unit == #str; }, \
        format_case(identity<::stdsharp::filesystem::type>{}, #next_str)}

                        STDSHARP_MAKE_PAIR(b, bits, ),
                        STDSHARP_MAKE_PAIR(B, bytes, b),
                        STDSHARP_MAKE_PAIR(KiB, kibibytes, B),
                        STDSHARP_MAKE_PAIR(KB, kilobytes, B),
                        STDSHARP_MAKE_PAIR(MiB, mebibytes, KiB),
                        STDSHARP_MAKE_PAIR(MB, megabytes, KB),
                        STDSHARP_MAKE_PAIR(GiB, gibibytes, MiB),
                        STDSHARP_MAKE_PAIR(GB, gigabytes, MB),
                        STDSHARP_MAKE_PAIR(TiB, tebibytes, GiB),
                        STDSHARP_MAKE_PAIR(TB, terabytes, GB),
                        STDSHARP_MAKE_PAIR(PiB, pebibytes, TiB),
                        STDSHARP_MAKE_PAIR(PB, petabytes, TB),
                        STDSHARP_MAKE_PAIR(EiB, exbibytes, PiB),
                        STDSHARP_MAKE_PAIR(EB, exabytes, PB)

#undef STDSHARP_MAKE_PAIR
                    );
                };

                if(locale_.use_locale)
#if __cpp_lib_format >= 201907L
                    ss.imbue(fc.locale());
#else
                    ss.imbue(fc.locale().template get<::std::locale>());
#endif
                else ss.imbue(::std::locale::classic());

                if(const auto precision_v = fmtsharp::get_arg(fc, precision); precision_v)
                    for(auto i = *precision_v; i != 0; --i) do_format();
                else
                    while(s.size() > 0)
                    {
                        if(current_unit.empty())
                        {
                            ss << s;
                            break;
                        }

                        do_format();
                    }

                return ::std::move(ss).str();
            }();

            if(
                [=, &formatted, &fc, align = align_]
                {
                    if(fill && width && align != fmtsharp::align_t::none)
                    {
                        const auto width_v = *width;

                        if(formatted.size() < width_v)
                        {
                            const auto fill_c = *fill;
                            const auto fill_size = width_v - formatted.size();

                            switch(align)
                            {
                            case fmtsharp::align_t::left:
                                ::std::ranges::copy(formatted, fc.out());
                                ::std::ranges::fill_n(fc.out(), fill_size, fill_c);
                                break;

                            case fmtsharp::align_t::right:
                                ::std::ranges::fill_n(fc.out(), fill_size, fill_c);
                                ::std::ranges::copy(formatted, fc.out());
                                break;

                            case fmtsharp::align_t::center:
                            {
                                const auto half_fill_s = fill_size / 2;
                                ::std::ranges::fill_n(fc.out(), half_fill_s, fill_c);
                                ::std::ranges::copy(formatted, fc.out());
                                ::std::ranges::fill_n(fc.out(), fill_size - half_fill_s, fill_c);
                            }
                            break;

                            default: break;
                            }

                            return false;
                        }
                    }

                    return true;
                }()
            )
                ::std::ranges::copy(formatted, fc.out());
            return fc.out();
        }
    };
}