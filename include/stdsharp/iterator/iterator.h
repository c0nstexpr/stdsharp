//

//
#pragma once

#include <iterator>

#include "../type_traits/core_traits.h"

namespace stdsharp
{
    template<::std::indirectly_readable T>
    using iter_const_reference_t =
#if __cpp_lib_ranges_as_const >= 202207L
        ::std::iter_const_reference_t<T>
#else
        ::std::common_reference_t<const ::std::iter_value_t<T>&&, ::std::iter_reference_t<T>>
#endif
        ;

    template<typename I>
    concept weakly_decrementable = ::std::movable<I> &&
        requires(I i) //
    {
        typename ::std::iter_difference_t<I>;
        requires ::std::signed_integral<::std::iter_difference_t<I>>; // clang-format off
        { --i } -> ::std::same_as<I&>; // clang-format on
        i--;
    };

    template<typename I>
    concept decrementable = ::std::regular<I> && weakly_decrementable<I> &&
        requires(I i) // clang-format off
    {
        { i-- } -> ::std::same_as<I>; // clang-format on
    };
}