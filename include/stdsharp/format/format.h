#pragma once

#include <sstream>
#include <variant>
#include <optional>
#include <numeric>

#include <range/v3/view/repeat_n.hpp>
#include <ctre.hpp>
#include <unicode-db.hpp>

#if __cpp_lib_format >= 201907L
    #include <format>
    #define FORMAT_NS ::std
#else
    #include <fmt/format.h>
    #include <fmt/xchar.h>
    #define FORMAT_NS ::fmt
#endif

#include "../cstdint/cstdint.h"
#include "../concepts/concepts.h"
#include "../details/prologue.h"

namespace stdsharp::fmt
{
    namespace details
    {
        template<typename CharT>
        using parse_context = FORMAT_NS::basic_format_parse_context<CharT>;


        template<typename OutputIt, typename CharT>
        using context = FORMAT_NS::basic_format_context<OutputIt, CharT>;

        template<typename CharT>
        using iter = typename FORMAT_NS::basic_format_parse_context<CharT>::iterator;

        template<::std::integral T>
        constexpr auto parse_integer(const auto& rng) noexcept
        {
            return ::std::accumulate(
                rng.begin(),
                rng.end(),
                T{0},
                [](const T v, const auto c) noexcept
                {
                    constexpr auto base = 10;
                    return v * base + static_cast<char>(c) - '0';
                } //
            );
        };
    }

    struct nested_arg_index
    {
        ::std::size_t value;

        template<typename Visitor, typename OutputIt, typename CharT>
        [[nodiscard]] constexpr decltype(auto) get_from_context(
            const details::context<OutputIt, CharT>& fc,
            Visitor&& vis //
        ) const
        {
            return FORMAT_NS::visit_format_arg(::std::forward<Visitor>(vis), fc.arg(value));
        }

        template<typename T, typename OutputIt, typename CharT>
            requires requires { true; }
        [[nodiscard]] constexpr decltype(auto)
            get_from_context(const details::context<OutputIt, CharT>& fc) const
        {
            return get_from_context(
                fc,
                []<typename U>(U&& u) noexcept -> ::std::optional<T>
                {
                    if constexpr(::std::convertible_to<U, T>)
                        return static_cast<T>(::std::forward<U>(u));
                    else
                        return ::std::nullopt;
                } //
            );
        }
    };

    template<typename T>
    using nested_spec = ::std::variant<::std::monostate, T, nested_arg_index>;

    namespace details
    {
        template<typename T>
        T nested_spec_like(nested_spec<T>);
    }

    template<typename ResultT = void, typename SpecT, typename OutputIt, typename CharT>
        requires requires(SpecT t) { details::nested_spec_like(t); }
    [[nodiscard]] constexpr auto get_arg(const details::context<OutputIt, CharT>& fc, SpecT&& spec)
    {
        using result_t = ::std::conditional_t<
            ::std::same_as<ResultT, void>,
            decltype(details::nested_spec_like(spec)),
            ResultT // clang-format off
        >; // clang-format on

        if(spec.valueless_by_exception() || ::std::holds_alternative<::std::monostate>(spec))
            return ::std::optional<result_t>{::std::nullopt};
        return ::std::visit(
            [&fc]<typename U>(U&& u) -> ::std::optional<result_t>
            {
                if constexpr(::std::same_as<::std::remove_cvref_t<U>, result_t>)
                    return ::std::optional<result_t>{::std::forward<U>(u)};
                else if constexpr(::std::same_as<::std::remove_cvref_t<U>, nested_arg_index>)
                    return ::std::forward<U>(u).template get_from_context<result_t>(fc);
                else
                    return ::std::nullopt;
            },
            spec //
        );
    }

    template<typename CharT>
    void parse_assert(const details::parse_context<CharT>& ctx)
    {
        const auto begin = ctx.begin();
        throw FORMAT_NS::format_error{
            FORMAT_NS::format(
                "invalid format:\n \"{}\"\n{}",
                ::std::basic_string_view{begin, ctx.end()},
                FORMAT_NS::format(
                    "{}^ Unexpected character here",
                    ::ranges::views::repeat_n(' ', begin - ctx.begin()) // clang-format off
                    )
            ) // clang-format on
        };
    }

    template<typename CharT, ::std::predicate<CharT> Predicate>
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

        throw FORMAT_NS::format_error{
            FORMAT_NS::format(
                "invalid format: \"{}\"\nEnd of string expected",
                ::std::basic_string_view{begin, ctx.end()} // clang-format off
            ) // clang-format on
        };
    }

    template<typename CharT>
    struct fill_spec
    {
        ::std::optional<CharT> fill;
    };

    template<typename CharT>
    [[nodiscard]] constexpr fill_spec<CharT> parse_fill_spec(details::parse_context<CharT>& ctx)
    {
        const auto [fill] = ::ctre::starts_with<".">(ctx);

        if(fill)
        {
            ctx.advance_to(fill.end());
            return {*fill.begin()};
        }
        return {::std::nullopt};
    }

    enum class align_t
    {
        none,
        left,
        right,
        center
    };

    template<::std::convertible_to<char> CharT>
    [[nodiscard]] constexpr auto parse_align_spec(details::parse_context<CharT>& ctx)
    {
        const auto [align] = ::ctre::starts_with<"[<^>]">(ctx);

        if(align)
        {
            ctx.advance_to(align.end());
            switch(static_cast<char>(*align.begin()))
            {
            case '<': return align_t::left; break;
            case '^': return align_t::center; break;
            case '>': return align_t::right; break;
            };
        }

        return align_t::none;
    }

    template<::std::integral IntType, ::std::convertible_to<char> CharT>
    [[nodiscard]] constexpr nested_spec<IntType>
        parse_nested_integer_spec(details::parse_context<CharT>& ctx)
    {
        const auto [_, ref, value] = ::ctre::starts_with<R"(\{(\d*)\}|(\d*))">(ctx);
        const auto end = _.end();

        if(ref)
        {
            ctx.advance_to(end);
            return nested_arg_index{
                ::std::ranges::empty(ref) ? //
                    ctx.next_arg_id() :
                    details::parse_integer<::std::size_t>(ref) //
            };
        }
        if(value)
        {
            ctx.advance_to(end);
            return details::parse_integer<::std::size_t>(value);
        }

        return {};
    }

    struct precision_spec
    {
        nested_spec<u64> precision{};
    };

    template<typename CharT>
    constexpr precision_spec parse_precision_spec(details::parse_context<CharT>& ctx)
    {
        const auto origin_begin = ctx.begin();
        const auto [dot] = ::ctre::starts_with<"\\.">(ctx);

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
        const auto [use_locale] = ::ctre::starts_with<"L">(ctx);
        if(use_locale)
        {
            ctx.advance_to(use_locale.end());
            return {true};
        }

        return {};
    }
}

#undef FORMAT_NS

#include "../details/epilogue.h"