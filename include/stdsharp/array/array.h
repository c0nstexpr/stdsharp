#pragma once

#include <array>
#include <algorithm>
#include <ranges>

namespace stdsharp
{
    template<::std::size_t N>
    struct range_to_array_fn
    {
        template<typename Rng, typename Proj = ::std::identity>
        constexpr auto operator()(Rng&& rng, Proj&& proj = {}) const
        {
            ::std::array<::std::projected<::std::ranges::iterator_t<Rng>, Proj>, N> arr{};

            ::std::ranges::copy(
                ::std::forward<Rng>(rng) | //
                    ::std::views::transform(::std::forward<Proj>(proj)) | //
                    ::std::views::take(N),
                arr.begin()
            );

            return arr;
        }
    };

    template<::std::size_t N>
    inline constexpr range_to_array_fn<N> range_to_array{};
}