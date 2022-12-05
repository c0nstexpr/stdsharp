//

//
#pragma once

#include <ranges>
#include <range/v3/range.hpp>
#include <range/v3/view.hpp>
#include <fmt/ranges.h>

#include "../type_traits/core_traits.h"

namespace stdsharp
{
    template<typename T>
    using const_iterator_t = decltype(::std::ranges::cbegin(::std::declval<T&>()));

    template<typename T>
    using const_sentinel_t = decltype(::std::ranges::cend(::std::declval<T&>()));

    template<typename T>
    using range_const_reference_t = add_const_lvalue_ref_t<::std::ranges::range_value_t<T>>;
}