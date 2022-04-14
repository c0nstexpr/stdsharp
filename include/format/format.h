#pragma once

#include <range/v3/view/repeat_n.hpp>
#include <string_view>
#if __cpp_lib_format >= 201907L
    #include <format>
#else
    #include <fmt/ostream.h>
#endif

#include "cassert/cassert.h"

namespace stdsharp
{
    namespace format =
#if __cpp_lib_format >= 201907L
        ::std
#else
        ::fmt
#endif
        ;

    template<typename CharT>
    constexpr void assert_fmt_parse_ctx(
        const format::basic_format_parse_context<CharT>& ctx,
        const typename format::basic_format_parse_context<CharT>::iterator current_it //
    )
    {
        if(::std::is_constant_evaluated()) unreachable();
        else
        {
            const auto& view = ::ranges::views::repeat_n(' ', current_it - ctx.begin());
            throw format::format_error{
                //
                format::format(
                    "invalid format:\n \"{}\"\n{}",
                    ::std::basic_string_view{ctx.begin(), ctx.end()},
                    format::format("{}^", view) //
                    ) //
            };
        }
    }

    template<typename CharT, ::std::predicate<CharT> Predicate>
    constexpr void validate_fmt_parse_ctx(
        const format::basic_format_parse_context<CharT>& ctx,
        const typename format::basic_format_parse_context<CharT>::iterator current_it,
        Predicate predicate //
    )
    {
        if(current_it == ctx.end() || !predicate(*current_it))
            assert_fmt_parse_ctx(ctx, current_it);
    }

    template<typename CharT>
    constexpr void validate_fmt_parse_ctx_not_end(
        const format::basic_format_parse_context<CharT>& ctx,
        const typename format::basic_format_parse_context<CharT>::iterator current_it //
    )
    {
        validate_fmt_parse_ctx(ctx, current_it, [](const auto) { return true; });
    }
}