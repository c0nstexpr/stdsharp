#pragma once

#include <filesystem>
#include <limits>
#include <ratio>

#include "default_operator.h"

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

    template<auto Num, auto Denom>
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
        static constexpr auto from_ratio(const auto factor, const ::std::ratio<N, D>) noexcept
        {
            return factor * N / D;
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
            value_(from_ratio(other.value_, ::std::ratio_divide<Period, period>{}))
        {
        }

        template<typename Period>
        [[nodiscard]] constexpr auto operator<=>(const space_size<Period> other) const noexcept
        {
            if constexpr(::std::ratio_greater_v<period, Period>)
                return value_ <=> from_ratio(other.value_, ::std::ratio_divide<Period, period>{});
            else
                return other.value_ <=> from_ratio(value_, ::std::ratio_divide<period, Period>{});
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
}