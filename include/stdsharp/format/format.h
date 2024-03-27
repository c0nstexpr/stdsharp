#pragma once

#include <ranges>
#include <variant>
#include <optional>
#include <numeric>
#include <format>

#include <ctre.hpp>
#include <unicode-db.hpp>

#include "../cstdint/cstdint.h"

namespace stdsharp
{
    namespace details
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
                [](const T v, const auto c) noexcept
                {
                    constexpr auto base = 10;
                    return v * base + static_cast<char>(c) - '0';
                }
            );
        };
    }

    struct nested_arg_index
    {
        std::size_t value;

        template<typename Visitor, typename OutputIt, typename CharT>
        [[nodiscard]] constexpr decltype(auto) get_from_context(
            const details::context<OutputIt, CharT>& fc,
            Visitor&& vis //
        ) const
        {
            return std::visit_format_arg(cpp_forward(vis), fc.arg(value));
        }

        template<typename T, typename OutputIt, typename CharT>
            requires requires { true; }
        [[nodiscard]] constexpr decltype(auto) get_from_context( //
            const details::context<OutputIt, CharT>& fc
        ) const
        {
            return get_from_context(
                fc,
                []<typename U>(U&& u) noexcept -> std::optional<T>
                {
                    if constexpr(std::convertible_to<U, T>) return static_cast<T>(cpp_forward(u));
                    else return std::nullopt;
                }
            );
        }
    };

    template<typename T>
    using nested_spec = std::variant<std::monostate, T, nested_arg_index>;

    namespace details
    {
        template<typename T>
        T nested_spec_like(nested_spec<T>);
    }

    template<typename ResultT = void, typename SpecT, typename OutputIt, typename CharT>
    [[nodiscard]] constexpr auto get_arg(const details::context<OutputIt, CharT>& fc, SpecT&& spec)
        requires requires { details::nested_spec_like(spec); }
    {
        using result_t = std::conditional_t<
            std::same_as<ResultT, void>,
            decltype(details::nested_spec_like(spec)),
            ResultT>;

        return spec.valueless_by_exception() || std::holds_alternative<std::monostate>(spec) ?
            std::optional<result_t>{std::nullopt} :
            std::visit(
                [&fc]<typename U>(U&& u) -> std::optional<result_t>
                {
                    if constexpr(std::same_as<std::remove_cvref_t<U>, result_t>)
                        return std::optional<result_t>{cpp_forward(u)};
                    else if constexpr(std::same_as<std::remove_cvref_t<U>, nested_arg_index>)
                        return cpp_forward(u).template get_from_context<result_t>(fc);
                    else return std::nullopt;
                },
                spec
            );
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

        throw std::format_error{std::format(
            "invalid format: \"{}\"\nEnd of string expected",
            std::basic_string_view{begin, ctx.end()}
        )};
    }

    template<typename CharT>
    struct fill_spec
    {
        std::optional<CharT> fill;
    };

    template<typename CharT>
    [[nodiscard]] constexpr fill_spec<CharT> parse_fill_spec(details::parse_context<CharT>& ctx)
    {
        const auto& fill = ctre::starts_with<".">(ctx);

        if(fill)
        {
            ctx.advance_to(fill.end());
            return {*fill.begin()};
        }
        return {std::nullopt};
    }

    enum class align_t : std::uint8_t
    {
        none,
        left,
        right,
        center
    };

    template<std::convertible_to<char> CharT>
    [[nodiscard]] constexpr auto parse_align_spec(details::parse_context<CharT>& ctx)
    {
        const auto& align = ctre::starts_with<"[<^>]">(ctx);

        if(align)
        {
            ctx.advance_to(align.end());
            switch(static_cast<char>(*align.begin()))
            {
            case '<': return align_t::left;
            case '^': return align_t::center;
            case '>': return align_t::right;
            };
        }

        return align_t::none;
    }

    template<std::integral IntType, std::convertible_to<char> CharT>
    [[nodiscard]] constexpr nested_spec<IntType>
        parse_nested_integer_spec(details::parse_context<CharT>& ctx)
    {
        const auto& captures = ctre::starts_with<R"(\{(\d*)\}|(\d*))">(ctx);
        const auto& ref = captures.template get<1>();
        const auto& value = captures.template get<2>();
        const auto end = captures.end();

        if(ref)
        {
            ctx.advance_to(end);
            return nested_arg_index{
                std::ranges::empty(ref) ? //
                    ctx.next_arg_id() :
                    details::parse_integer<std::size_t>(ref)
            };
        }
        if(value)
        {
            ctx.advance_to(end);
            return details::parse_integer<std::size_t>(value);
        }

        return {};
    }

    struct precision_spec
    {
        nested_spec<u64> precision;
    };

    template<typename CharT>
    constexpr precision_spec parse_precision_spec(details::parse_context<CharT>& ctx)
    {
        const auto origin_begin = ctx.begin();
        const auto& dot = ctre::starts_with<"\\.">(ctx);

        if(dot)
        {
            const auto dot_end = dot.end();

            ctx.advance_to(dot_end);

            return {parse_nested_integer_spec<u64>(ctx)};
        }

        ctx.advance_to(origin_begin);
        return {};
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