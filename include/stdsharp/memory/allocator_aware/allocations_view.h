#pragma once

#include "allocation.h"

namespace stdsharp::allocator_aware::cpo
{
    inline namespace cpo_impl
    {
        void allocations_view(auto&&) = delete;

        template<typename Allocator>
        struct default_allocations_view_fn
        {
            template<std::ranges::input_range T>
                requires std::ranges::output_range<T, allocation<Allocator>> &&
                std::convertible_to<std::ranges::range_reference_t<T>, allocation<Allocator>>
            [[nodiscard]] constexpr auto operator()(T&& t) const noexcept
            {
                return t | views::cast<allocation<Allocator>&>;
            }
        };
    }

    template<typename Allocator>
    inline constexpr allocations_view_fn<Allocator> allocations_view;

}