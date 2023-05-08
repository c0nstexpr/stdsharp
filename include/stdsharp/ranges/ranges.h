//

//
#pragma once

#include <ranges>
#include "../iterator/iterator.h"

#include <range/v3/range.hpp>
#include <range/v3/view.hpp>
#include <fmt/ranges.h>

#include "../utility/utility.h"

namespace stdsharp
{
    template<typename T>
    using const_iterator_t = decltype(::std::ranges::cbegin(::std::declval<T&>()));

    template<typename T>
    using const_sentinel_t = decltype(::std::ranges::cend(::std::declval<T&>()));

    template<typename T>
    using range_const_reference_t = iter_const_reference_t<::std::ranges::iterator_t<T>>;

    template<typename T>
    using forwarding_views = ::std::ranges::transform_view<T, forward_like_fn<T>>;

    namespace views
    {
        inline constexpr nodiscard_invocable forwarding =
            []<typename T>(T&& t) noexcept(nothrow_constructible_from<forwarding_views<T>, T>)
        {
            return forwarding_views<T>{cpp_forward(t)}; //
        };
    }
}