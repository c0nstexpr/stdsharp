//
// Created by BlurringShadow on 2021-9-27.
//

#pragma once
#include <ranges>
#include <range/v3/range.hpp>
#include <range/v3/view.hpp>

#include "functional/invocable_obj.h"
#include "type_traits/type_traits.h"

namespace stdsharp
{
    namespace ranges
    {
        template<typename T>
        using const_iterator_t = decltype(::std::ranges::cbegin(::std::declval<T&>()));

        template<typename T>
        using range_const_reference_t =
            type_traits::add_const_lvalue_ref_t<::std::ranges::range_value_t<T>>;

    }
}
