// Created by BlurringShadow at 2021-02-28-下午 10:45

#pragma once

#include <charconv>
#include "utility_core.h"

namespace c0nstexpr::utility
{
    namespace details
    {
        struct to_string_cpo
        {
            template<typename T>
            [[nodiscard]] constexpr auto operator()(const T& t) const
            {
                return fmt::format("{}", t);
            }

            template<typename T> requires requires(T t) { to_string(t); }
            [[nodiscard]] constexpr auto operator()(const T& t) const { return to_string(t); }
        };

        template<typename T>
        concept has_from_string_cpo = 
        requires(const std::string_view str, T t) { from_string(str, t); };
    }

    inline constexpr details::to_string_cpo to_string;

    template<typename T>
    inline constexpr auto from_string =
        [](
        const std::string_view str
    ) [[nodiscard]] -> T
    {
        if constexpr(concepts::constructible_from<T, std::string_view>)
            return {str};
        else if constexpr(concepts::constructible_from<T, const char*>)
            return {str.data()};
        else if constexpr(concepts::constructible_from<T, const char*, const char*>)
            return {str.data(), str.data() + str.size()};
        else if constexpr(std::constructible_from<T, const char*, std::size_t>)
            return {str.data(), str.size()};
        else
        {
            T t{};

            if constexpr(details::has_from_string_cpo<T>) from_string(str, t);
            else
            {
                using std::from_chars;
                if(from_chars(str.data(), str.data() + str.size(), t).ec != std::errc{})
                    throw std::invalid_argument("");
            }

            return t;
        }
    };
}
