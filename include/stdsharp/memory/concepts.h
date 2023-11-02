#pragma once

#include <ranges>

namespace stdsharp
{
    // These concepts allow some operations on iterators and sentinels to throw exceptions, e.g.
    // operations on invalid values.

    // no exceptions are thrown from increment, copy construction, move construction, copy
    // assignment, move assignment, or indirection through valid iterators
    template<typename I>
    concept nothrow_input_iterator =
        std::input_iterator<I> && std::is_lvalue_reference_v<std::iter_reference_t<I>> &&
        std::same_as<std::remove_cvref_t<std::iter_reference_t<I>>, std::iter_value_t<I>>;

    // no exceptions are thrown from copy construction, move construction, copy assignment, move
    // assignment, or comparisons between valid values of type I and S
    template<typename S, typename I>
    concept nothrow_sentinel_for = std::sentinel_for<S, I>;

    template<typename I>
    concept nothrow_forward_iterator =
        nothrow_input_iterator<I> && std::forward_iterator<I> && nothrow_sentinel_for<I, I>;

    // no exceptions are thrown from calls to ranges::begin and ranges::end on an object of type R.
    template<typename R>
    concept nothrow_input_range = std::ranges::range<R> && //
        nothrow_input_iterator<std::ranges::iterator_t<R>> &&
        nothrow_sentinel_for<std::ranges::sentinel_t<R>, std::ranges::iterator_t<R>>;

    template<typename R>
    concept nothrow_forward_range =
        nothrow_input_range<R> && nothrow_forward_iterator<std::ranges::iterator_t<R>>;
}