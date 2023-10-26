#pragma once

#include "allocation.h"
#include <ranges>

namespace stdsharp::allocator_aware
{
    inline namespace cpo_impl
    {
        template<typename Rng, typename Allocator>
        concept allocations_view_concept = allocations_view<Rng, Allocator>;

        template<typename Rng, typename Allocator>
        concept callocations_view_concept = callocations_view<Rng, Allocator>;

        void allocations_view(auto&&) = delete;

        template<typename Allocator>
        struct default_allocations_view_fn
        {
            template<allocations_view_concept<Allocator> T>
            [[nodiscard]] constexpr auto operator()(T&& t) const noexcept
            {
                return t | views::cast<allocation<Allocator>&>;
            }

            template<typename T>
            [[nodiscard]] constexpr auto operator()(T&& t) const noexcept
                requires requires() { true; }
            {
                return t |
                    std::views::transform(
                           [](auto&& tuple)
                           {
                               return allocation<Allocator>{
                                   cpo::get_element<0>(cpp_forward(tuple)),
                                   cpo::get_element<1>(cpp_forward(tuple))
                               };
                           }
                    );
                ;
            }
        };

        template<typename Allocator>
        struct custom_allocations_view_fn
        {
            template<typename T>
            [[nodiscard]] constexpr decltype(auto) operator()(T && t) const noexcept
                requires requires(decltype(allocations_view(t)) view) {
                    requires std::ranges::input_range<decltype(view)>;
                    requires std::ranges::output_range<decltype(view), allocation<Allocator>>;
                    requires std::convertible_to<
                        std::ranges::range_reference_t<decltype(view)>,
                        allocation<Allocator>>;
                }
            {
                return allocations_view(t);
            }
        };
    }

    template<typename Allocator>
    inline constexpr as_allocations_view_fn<Allocator> as_allocations_view;

}