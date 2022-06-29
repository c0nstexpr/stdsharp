//
// Created by BlurringShadow on 2021-11-15.
//

#pragma once

#include <iterator>

#include "../type_traits/core_traits.h"
#include "../concepts/concepts.h"

namespace stdsharp::iterator
{
    template<typename T>
    concept referenceable = requires
    {
        typename ::std::add_lvalue_reference<T>;
    };

    template<typename T>
    concept dereferenceable = requires(T& t)
    { // clang-format off
        { *t } -> referenceable; // clang-format on
    };

    namespace details
    {
        template<typename T>
        struct iter_const_reference;

        template<typename T>
            requires requires { typename ::std::iter_value_t<T>; }
        struct iter_const_reference<T>
        {
            using type = typename type_traits::add_const_lvalue_ref_t<::std::iter_value_t<T>>;
        };

        template<typename T>
            requires requires
            {
                typename ::std::iter_value_t<T>;
                typename T::const_reference;
            }
        struct iter_const_reference<T>
        {
            using type = typename T::const_reference;
        };
    }

    template<typename T>
    using iter_const_reference_t = typename details::iter_const_reference<T>;

    template<typename I>
    concept weakly_decrementable = ::std::movable<I> && requires(I i)
    {
        typename ::std::iter_difference_t<I>;
        requires ::std::signed_integral<::std::iter_difference_t<I>>; // clang-format off
        { --i } -> ::std::same_as<I&>; // clang-format on
        i--;
    };

    template<typename I>
    concept decrementable = ::std::regular<I> && weakly_decrementable<I> && requires(I i)
    { // clang-format off
        { i-- } -> ::std::same_as<I>; // clang-format on
    };
}