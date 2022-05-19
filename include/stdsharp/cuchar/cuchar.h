#include <cstddef>
#include <cstdlib>
#include <cuchar>
#include <stdexcept>
#include <string>

#include "../functional/cpo.h"
#include "../pattern_match.h"

namespace stdsharp
{
    template<typename CharT>
    struct encode_to_string_fn
    {
        auto operator()(CharT character) const requires(
            !functional::cpo_invocable<encode_to_string_fn, CharT> &&
            concepts::same_as_any<CharT, char8_t, char16_t, char32_t, wchar_t> //
        )
        {
            ::std::mbstate_t mb = std::mbstate_t();
            ::std::string res(MB_CUR_MAX, '\0');

            res.resize( //
                constexpr_pattern_match::from_type<CharT>(
                    [&](const ::std::type_identity<wchar_t>)
                    {
                        return ::std::wcrtomb(res.data(), character, &mb); // NOLINT(*-mt-unsafe)
                    },
#if __cpp_lib_char8_t >= 201907L
    #ifndef __clang__
                    [&](const ::std::type_identity<char8_t>)
                    {
                        return ::std::c8rtomb(res.data(), character, &mb); //
                    },
    #endif
#endif
                    [&](const ::std::type_identity<char16_t>)
                    {
                        return ::std::c16rtomb(res.data(), character, &mb); //
                    },
                    [&](const ::std::type_identity<char32_t>)
                    {
                        return ::std::c32rtomb(res.data(), character, &mb); //
                    } // clang-format off
                ) // clang-format on
            );

            if(errno == EILSEQ) throw ::std::runtime_error("invalid character");
        }

        template<typename... Args>
            requires functional::cpo_invocable<encode_to_string_fn, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return functional::cpo_invoke(*this, ::std::forward<Args>(args)...);
        }
    };

    template<>
    struct encode_to_string_fn<char>
    {
        ::std::string operator()(char character) const { return {character}; }
    };
}