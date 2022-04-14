#pragma once

#include <filesystem>
#include <ratio>

#include "format/format.h"
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

    template<::std::uintmax_t Num, ::std::uintmax_t Denom>
        requires(!::std::same_as<::std::ratio<Num, Denom>, typename ::std::ratio<Num, Denom>::type>)
    class [[nodiscard]] space_size<::std::ratio<Num, Denom>> :
        public space_size<typename ::std::ratio<Num, Denom>::type>
    {
    };

    template<::std::uintmax_t Num, ::std::uintmax_t Denom>
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
        static constexpr auto cast_from(const auto factor, const ::std::ratio<N, D>) noexcept
        {
            return factor * Denom * N / (Num * D);
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
            value_(cast_from(other.value_, Period{}))
        {
        }

        template<typename Period>
        [[nodiscard]] constexpr auto operator<=>(const space_size<Period> other) const noexcept
        {
            if constexpr(::std::ratio_greater_equal_v<period, Period>)
                return value_ <=> cast_from(other.value_, Period{});
            else
                return other <=> *this;
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

    template<typename CharT, typename Traits>
    auto& operator<<(
        std::basic_ostream<CharT, Traits>& os,
        const concepts::same_as_any<
            bits,
            bytes,
            kilobytes,
            megabytes,
            gigabytes,
            terabytes,
            petabytes,
            exabytes,
            kibibytes,
            mebibytes,
            gibibytes,
            tebibytes,
            pebibytes,
            exbibytes // clang-format off
        >
        auto size // clang-format on
    )
    {
        constexpr_pattern_match::from_type<::std::remove_const_t<decltype(size)>>( //
            [&](const ::std::type_identity<bits>) { os << size.size() << "b"; },
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
#else
namespace fmt
#endif
{
    template<typename Period, typename CharT>
        requires requires(
            ::stdsharp::filesystem::space_size<Period> s,
            ::std::basic_stringstream<CharT> ss //
        )
        {
            ss << s;
        }
    struct formatter<::stdsharp::filesystem::space_size<Period>, CharT>
    {
    private:
        using space_size = ::stdsharp::filesystem::space_size<Period>;

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
                        [](const CharT c) { return static_cast<char>(c); } //
                    );
                    return from_unit;
                }();

                ::std::string_view current_unit{
                    from_unit.begin(), //
                    ::std::ranges::find(from_unit, char{}) //
                };
                ::std::basic_stringstream<CharT> ss;

                const auto do_format = [&current_unit, &ss, &s]
                {
                    const auto format_case =
                        [&]<typename SpaceSize>( // clang-format off
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
                            //
                            [](const ::std::string_view unit) noexcept { return unit.empty(); },
                            [](const ::std::string_view)
                            { throw format_error{"Precision exceeded"}; } //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "b"; },
                            format_case(identity<::stdsharp::filesystem::bits>{}, "")
                            //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "B"; },
                            format_case(
                                identity<::stdsharp::filesystem::bytes>{},
                                "b") //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "KiB"; },
                            format_case(
                                identity<::stdsharp::filesystem::kibibytes>{},
                                "B") //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "KB"; },
                            format_case(
                                identity<::stdsharp::filesystem::kilobytes>{},
                                "B") //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "MiB"; },
                            format_case(
                                identity<::stdsharp::filesystem::mebibytes>{},
                                "KiB") //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "MB"; },
                            format_case(
                                identity<::stdsharp::filesystem::megabytes>{},
                                "KB") //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "GiB"; },
                            format_case(
                                identity<::stdsharp::filesystem::gibibytes>{},
                                "MiB") //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "GB"; },
                            format_case(
                                identity<::stdsharp::filesystem::gigabytes>{},
                                "MB") //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "TiB"; },
                            format_case(
                                identity<::stdsharp::filesystem::tebibytes>{},
                                "GiB") //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "TB"; },
                            format_case(
                                identity<::stdsharp::filesystem::terabytes>{},
                                "GB") //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "PiB"; },
                            format_case(
                                identity<::stdsharp::filesystem::pebibytes>{},
                                "TiB") //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "PB"; },
                            format_case(
                                identity<::stdsharp::filesystem::petabytes>{},
                                "TB") //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "EiB"; },
                            format_case(
                                identity<::stdsharp::filesystem::exbibytes>{},
                                "PiB") //
                        },
                        ::std::pair{
                            //
                            [](const ::std::string_view unit) noexcept { return unit == "EB"; },
                            format_case(
                                identity<::stdsharp::filesystem::exabytes>{},
                                "PB") //
                        } //
                    );
                };

                if(locale_.use_locale)
#if __cpp_lib_format >= 201907L
                    ss.imbue(fc.locale());
#else
                    ss.imbue(fc.locale().template get<::std::locale>());
#endif
                else
                    ss.imbue(::std::locale::classic());

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

                return ss.str();
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
                }() //
            )
                ::std::ranges::copy(formatted, fc.out());
            return fc.out();
        }
    };
}