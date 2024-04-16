#pragma once

#include "../namespace_alias.h"

#include <algorithm>
#include <array>
#include <string_view>

namespace stdsharp::literals
{
    template<std::size_t Size>
    struct ltr : std::array<char, Size>
    {
    private:
        using array_t = const char (&)[Size]; // NOLINT(*-avoid-c-arrays)

    public:
        using base = std::array<char, Size>;
        using base::base;

        constexpr ltr(array_t arr) noexcept { std::ranges::copy(arr, base::begin()); }

        constexpr ltr& operator=(array_t arr) noexcept
        {
            std::ranges::copy(arr, base::begin());
            return *this;
        }

        [[nodiscard]] constexpr operator std::string_view() const noexcept
        {
            return {base::data(), Size - 1};
        }

        [[nodiscard]] constexpr auto to_string_view() const noexcept
        {
            return static_cast<std::string_view>(*this);
        }
    };

    template<std::size_t Size>
    ltr(const char (&)[Size]) -> ltr<Size>; // NOLINT(*-avoid-c-arrays)

    template<ltr Ltr>
    [[nodiscard]] constexpr auto operator""_ltr() noexcept
    {
        return Ltr;
    }
}