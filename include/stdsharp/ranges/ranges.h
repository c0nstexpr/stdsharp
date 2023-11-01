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
    concept range_copyble = std::ranges::input_range<In> &&
        std::ranges::output_range<Out, std::ranges::range_reference_t<Out>> &&
        std::indirectly_copyable<std::ranges::iterator_t<In>, std::ranges::iterator_t<Out>>;

    namespace views
    {
        inline constexpr nodiscard_invocable forwarding{
            []<typename T>(T&& t) noexcept
            {
                return t | std::ranges::views::transform(forward_like<T>); //
            }
        };

        template<typename U>
        inline constexpr nodiscard_invocable cast{std::ranges::views::transform(cast_to<U>)};
    }
}