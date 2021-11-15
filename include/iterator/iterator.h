#include <iterator>
#include <concepts/concepts.h>

namespace stdsharp::iterator
{
    template<typename I>
    concept weakly_decrementable = ::std::movable<I> && requires(I i)
    {
        typename ::std::iter_difference_t<I>;
        requires concepts::signed_integral<::std::iter_difference_t<I>>; // clang-format off
        { --i } -> ::std::same_as<I&>; // clang-format on
        i--;
    };

    template<typename I>
    concept decrementable = ::std::regular<I> && weakly_decrementable<I> && requires(I i)
    { // clang-format off
        { i-- } -> ::std::same_as<I>; // clang-format on
    };
}