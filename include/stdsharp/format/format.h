#pragma once

#include "../cstdint/cstdint.h"
#include "../functional/operations.h"

#include <ctre/../ctre.hpp>

#include <format>
#include <numeric>
#include <optional>
#include <ranges>
#include <unicode-db.hpp>
#include <variant>

namespace stdsharp::details
{
    template<typename CharT>
    using parse_context = std::basic_format_parse_context<CharT>;

    template<typename OutputIt, typename CharT>
    using context = std::basic_format_context<OutputIt, CharT>;

    template<typename CharT>
    using iter = typename std::basic_format_parse_context<CharT>::iterator;

    template<std::integral T>
    constexpr auto parse_integer(const auto& rng) noexcept
    {
        return std::accumulate(
            rng.begin(),
            rng.end(),
            T{0},
            []<typename U>(const T v, const U c)
            {
                for(const char char_num : "0123456789")
                    if(c == U{char_num})
                        return v * 10 + (char_num - '0'); // NOLINT(*-magic-numbers)

                throw std::format_error{"invalid integer num"};
            }
        );
    };
}

namespace stdsharp
{
    template<typename T, typename OutputIt, typename CharT, typename Fn = identity_with_fn<const T&>>
    [[nodiscard]] constexpr const T&
        get_fmt_context_nested(const std::size_t value, const details::context<OutputIt, CharT>& fc)
    {
        auto& arg = fc.arg(value);

        return
#if __cpp_lib_format >= 202306L
            arg.visit(Fn{})
#else
            std::visit_format_arg(Fn{}, arg)
#endif
                ;
    }

    template<typename CharT>
    [[noreturn]] void parse_assert(const details::parse_context<CharT>& ctx)
    {
        const auto begin = ctx.begin();
        throw std::format_error{std::format(
            "invalid format:\n \"{}\"\n{}",
            std::basic_string_view{begin, ctx.end()},
            std::format(
                "{}^ Unexpected character here",
                std::views::repeat(' ', begin - ctx.begin())
            )
        )};
    }

    template<typename CharT, std::predicate<CharT> Predicate>
    constexpr void parse_validate(const details::parse_context<CharT>& ctx, Predicate predicate)
    {
        const auto begin = ctx.begin();
        if(begin == ctx.end() || !predicate(*begin)) parse_assert(ctx, begin);
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
    [[nodiscard]] constexpr auto parse_fill_spec(details::parse_context<CharT>& ctx)
    {
        std::optional<fill_and_align_spec<CharT>> spec;

        const auto& [whole, fill, align] = ctre::starts_with<"([^<^>])([<^>])">(ctx);

        if(!whole) return spec;

        ctx.advance_to(whole.end());

        if(!align) return spec;

        spec.emplace();

        {
            auto& align_v = spec->align;

            if(const auto& align_char = *align.begin(); align_char == CharT{'<'})
                align_v = align_t::left;
            else if(align_char == CharT{'^'}) align_v = align_t::center;
            else if(align_char == CharT{'>'}) align_v = align_t::right;
            else throw std::format_error{"invalid align specifier"};
        }

        if(fill) spec->fill = *fill.begin();

        return spec;
    }

    template<std::integral IntType, std::constructible_from<char> CharT>
    [[nodiscard]] constexpr IntType parse_nested_integer_spec(details::parse_context<CharT>& ctx)
    {
        const auto& [whole, ref, value] = ctre::starts_with<R"(\{(\d*)\}|(\d*))">(ctx);

        if(!whole) return {};

        ctx.advance_to(whole.end());

        if(ref)
            return get_fmt_context_nested<IntType>(
                std::ranges::empty(ref) ? //
                    ctx.next_arg_id() :
                    details::parse_integer<std::size_t>(ref),
                ctx
            );

        if(value) return details::parse_integer<IntType>(value);

        return {};
    }

    template<std::integral IntType, typename CharT>
    constexpr std::optional<IntType> parse_precision_spec(details::parse_context<CharT>& ctx)
    {
        const auto origin_begin = ctx.begin();
        const auto& dot = ctre::starts_with<"\\.">(ctx);

        if(!dot) return {};

        ctx.advance_to(dot.end());

        return parse_nested_integer_spec<IntType>(ctx);
    }

    struct locale_spec
    {
        bool use_locale{};
    };

    template<typename CharT>
    constexpr locale_spec parse_locale_spec(details::parse_context<CharT>& ctx)
    {
        const auto& use_locale = ctre::starts_with<"L">(ctx);
        if(use_locale)
        {
            ctx.advance_to(use_locale.end());
            return {true};
        }

        return {};
    }
}