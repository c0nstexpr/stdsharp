#pragma once

#include "../functional/sequenced_invocables.h"

#include <ctre.hpp>
#include <unicode-db.hpp>

#include <format>
#include <numeric>
#include <optional>
#include <ranges>

namespace stdsharp::details
{
    template<typename CharT>
    using parse_context = std::basic_format_parse_context<CharT>;

    template<typename OutputIt, typename CharT>
    using context = std::basic_format_context<OutputIt, CharT>;

    template<std::unsigned_integral T>
    constexpr auto parse_integer(const auto& rng) noexcept
    {
        static const auto& char_num_range = []
        {
            using char_t = std::ranges::range_value_t<decltype(rng)>;

            std::array rng{
                char_t{'0'},
                char_t{'1'},
                char_t{'2'},
                char_t{'3'},
                char_t{'4'},
                char_t{'5'},
                char_t{'6'},
                char_t{'7'},
                char_t{'8'},
                char_t{'9'}
            };

            std::ranges::sort(rng);

            return rng;
        }();

        return std::accumulate(
            rng.begin(),
            rng.end(),
            T{0},
            []<typename U>(T v, const U& c)
            {
                const auto& found = std::ranges::lower_bound(char_num_range, c);

                if(found == char_num_range.end()) throw std::format_error{"invalid integer num"};

                v *= 10; // NOLINT(*-magic-numbers)
                v += found - char_num_range.begin();

                return v;
            }
        );
    };
}

namespace stdsharp
{
    template<typename T = void>
    struct visit_fmt_arg_fn
    {
        template<typename Visitor, typename OutputIt, typename CharT>
        [[nodiscard]] constexpr decltype(auto) operator()(
            const std::size_t value,
            Visitor visitor,
            const details::context<OutputIt, CharT>& fc
        )
        {
            if constexpr(const auto& arg = fc.arg(value); std::same_as<T, void>)
#if __cpp_lib_format >= 202306L
                return arg.visit(visitor);
            else return arg.template visit<T>(visitor);
#else
                return std::visit_format_arg(visitor, arg);
            else return std::visit_format_arg<T>(visitor, arg);
#endif
        }
    };

    template<typename T = void>
    inline constexpr visit_fmt_arg_fn<T> visit_fmt_arg{};

    template<typename CharT>
    [[noreturn]] void parse_assert(const details::parse_context<CharT>& ctx)
    {
        const auto begin = ctx.begin();
        throw std::format_error{
            std::format(
                "invalid format:\n \"{}\"\n{}",
                std::basic_string_view{begin, ctx.end()},
                std::format(
                    "{}^ Unexpected character here",
                    std::views::repeat(' ', begin - ctx.begin())
                )
            ) //
        };
    }

    template<typename CharT, std::predicate<CharT> Predicate>
    constexpr void parse_validate(const details::parse_context<CharT>& ctx, Predicate predicate)
    {
        if(const auto begin = ctx.begin(); begin == ctx.end() || !predicate(*begin))
            parse_assert(ctx, begin);
    }

    template<typename CharT>
    constexpr void parse_not_end_assert(const details::parse_context<CharT>& ctx)
    {
        parse_validate(ctx, [](const auto) { return true; });
    }

    template<typename CharT>
    constexpr void parse_end_assert(const details::parse_context<CharT>& ctx)
    {
        const auto begin = ctx.begin();

        if(begin == ctx.end() || *begin == '}') return;

        throw std::format_error{
            std::format(
                "invalid format: \"{}\"\nEnd of string expected",
                std::basic_string_view{begin, ctx.end()}
            ) //
        };
    }

    enum class align_t : std::uint8_t
    {
        none,
        left,
        right,
        center
    };

    template<typename CharT>
    struct fill_and_align_spec
    {
        align_t align{};
        CharT fill = ' ';
    };

    template<std::constructible_from<char> CharT>
    [[nodiscard]] constexpr fill_and_align_spec<CharT>
        parse_fill_and_align_spec(details::parse_context<CharT>& ctx)
    {
        const auto& [whole, fill, align] = ctre::starts_with<"([^<^>])([<^>])">(ctx);

        if(!whole) return {};

        fill_and_align_spec<CharT> spec;

        if(fill) spec.fill = *fill.begin();

        {
            auto& align_v = spec.align;

            if(const auto& align_char = *align.begin(); align_char == CharT{'<'})
                align_v = align_t::left;
            else if(align_char == CharT{'^'}) align_v = align_t::center;
            else if(align_char == CharT{'>'}) align_v = align_t::right;
            else throw std::format_error{"invalid align specifier"};
        }

        ctx.advance_to(whole.end());

        return spec;
    }

    template<std::integral IntType, std::constructible_from<char> CharT>
    [[nodiscard]] constexpr std::optional<IntType>
        parse_nested_integer_spec(details::parse_context<CharT>& ctx)
    {
        const auto& [whole, ref, value] = ctre::starts_with<R"(\{(\d*)\}|(\d*))">(ctx);

        if(!whole) return {};

        const auto& int_v = ref ? //
            visit_fmt_arg<IntType>(
                std::ranges::empty(ref) ? //
                    ctx.next_arg_id() :
                    details::parse_integer<std::size_t>(ref),
                sequenced_invocables{
                    [](const IntType& v) { return v; },
                    [] [[noreturn]] (const auto&)
                    {
                        throw std::format_error{"invalid integer character"}; //
                    }
                },
                ctx
            ) :
            value ? details::parse_integer<IntType>(value) : IntType{};

        ctx.advance_to(whole.end());

        return {int_v};
    }

    template<std::integral IntType, std::constructible_from<char> CharT>
    constexpr std::optional<IntType> parse_precision_spec(details::parse_context<CharT>& ctx)
    {
        const auto& dot = ctre::starts_with<"\\.">(ctx);

        if(!dot) return {};

        ctx.advance_to(dot.end());

        const auto& int_v = parse_nested_integer_spec<IntType>(ctx);

        return int_v ? int_v : throw std::format_error{"invalid precision specify"};
    }

    template<typename CharT>
    constexpr bool parse_locale_spec(details::parse_context<CharT>& ctx)
    {
        const auto& use_locale = ctre::starts_with<"L">(ctx);

        if(!use_locale) return false;

        ctx.advance_to(use_locale.end());

        return true;
    }
}