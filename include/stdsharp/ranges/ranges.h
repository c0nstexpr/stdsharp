//

//
#pragma once

#include <ranges>

#include <range/v3/range.hpp>
#include <range/v3/view.hpp>

#include "../iterator/iterator.h"
#include "../functional/invocables.h"
#include "../utility/utility.h"

namespace stdsharp
{
    template<typename T>
    using const_iterator_t =
#if __cpp_lib_ranges_as_const >= 202207L
        std::ranges::const_iterator_t<T>;
#else
        decltype(std::ranges::cbegin(std::declval<T&>()));
#endif

    template<typename T>
    using const_sentinel_t =
#if __cpp_lib_ranges_as_const >= 202207L
        std::ranges::const_sentinel_t<T>;
#else
        decltype(std::ranges::cend(std::declval<T&>()));
#endif

    template<typename T>
    using range_const_reference_t =
#if __cpp_lib_ranges_as_const >= 202207L
        std::ranges::range_const_reference_t<T>;
#else
        iter_const_reference_t<std::ranges::iterator_t<T>>;
#endif

    template<typename T>
    using forwarding_view = std::ranges::transform_view<T, forward_like_fn<T>>;

    template<typename T, typename U>
    using cast_view = std::ranges::transform_view<T, cast_to_fn<U>>;

    template<typename T>
    concept constant_iterator =
        std::input_iterator<T> && std::same_as<iter_const_reference_t<T>, std::iter_reference_t<T>>;

    template<typename T>
    concept constant_range =
#if __cpp_lib_ranges_as_const >= 202207L
        std::ranges::constant_range<T>;
#else
        std::ranges::input_range<T> && constant_iterator<std::ranges::iterator_t<T>>;
#endif

    template<typename Out, typename In>
    concept range_movable = std::ranges::input_range<In> &&
        std::ranges::output_range<Out, std::ranges::range_rvalue_reference_t<In>> &&
        std::indirectly_movable<std::ranges::iterator_t<In>, std::ranges::iterator_t<Out>>;

    template<typename Out, typename In>
    concept range_copyable = std::ranges::input_range<In> &&
        std::ranges::output_range<Out, std::ranges::range_reference_t<Out>> &&
        std::indirectly_copyable<std::ranges::iterator_t<In>, std::ranges::iterator_t<Out>>;

    namespace views
    {
        template<typename T>
        inline constexpr auto forwarding = std::ranges::views::transform(forward_like<T>);

        template<typename U>
        inline constexpr auto cast = std::ranges::views::transform(cast_to<U>);
    }

    inline constexpr struct is_iter_in_fn
    {
        template<typename I, std::sentinel_for<I> S>
            requires std::sentinel_for<I, I>
        constexpr bool operator()(I begin, const S& end, const I& in) const noexcept
        {
            if(!std::is_constant_evaluated()) return begin <= in && in < end;

            for(; begin != end; ++begin)
                if(begin == in) return true;
            return false;
        }

        template<std::ranges::range R>
            requires std::sentinel_for<std::ranges::iterator_t<R>, std::ranges::iterator_t<R>>
        constexpr bool operator()(R&& r, const const_iterator_t<R>& in) const noexcept
        {
            return (*this)(std::ranges::cbegin(r), std::ranges::cend(r), in);
        }
    } is_iter_in{};
}