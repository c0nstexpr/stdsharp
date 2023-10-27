#pragma once

#include "../allocator_traits.h"
#include "../pointer_traits.h"

namespace stdsharp::allocator_aware
{
    template<typename T, typename Alloc>
    concept callocation = std::ranges::sized_range<T> && requires(const T& t) {
        {
            std::ranges::cbegin(t)
        } noexcept -> std::same_as<allocator_const_pointer<Alloc>>;
        {
            std::ranges::size(t)
        } noexcept -> std::same_as<allocator_size_type<Alloc>>;
    };

    template<typename T, typename Alloc>
    concept allocation = callocation<T, Alloc> && //
        requires(const T& ct,
                 std::decay_t<T> decay_t,
                 allocator_pointer<Alloc> p,
                 allocator_size_type<Alloc> size) {
            {
                std::ranges::begin(ct)
            } noexcept -> std::same_as<decltype(p)>;

            requires nothrow_default_initializable<decltype(decay_t)>;
            requires nothrow_constructible_from<decltype(decay_t), decltype(p), decltype(size)>;
            requires nothrow_movable<decltype(decay_t)>;
            requires[]
            {
                struct local
                {
                    constexpr decltype(p) begin() const noexcept;
                    constexpr decltype(p) end() const noexcept;
                    constexpr decltype(size) size() const noexcept;
                };

                return nothrow_constructible_from<decltype(decay_t), local>;
            }
            ();
        };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    struct allocation_data_fn
    {
        constexpr auto operator()(const callocation<Alloc> auto& rng) const noexcept
            requires std::same_as<T, typename Alloc::value_type>
        {
            return std::ranges::begin(cpp_forward(rng));
        }

        constexpr auto operator()(const callocation<Alloc> auto& rng) const noexcept
        {
            return pointer_cast<T>(std::ranges::begin(cpp_forward(rng)));
        }
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    static constexpr allocation_data_fn<Alloc, T> allocation_data{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    struct allocation_cdata_fn
    {
        constexpr auto operator()(const callocation<Alloc> auto& rng) const noexcept
            requires std::same_as<T, typename Alloc::value_type>
        {
            return std::ranges::cbegin(cpp_forward(rng));
        }

        constexpr auto operator()(const callocation<Alloc> auto& rng) const noexcept
        {
            return pointer_cast<T>(std::ranges::cbegin(cpp_forward(rng)));
        }
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    static constexpr allocation_cdata_fn<Alloc, T> allocation_cdata{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    struct allocation_get_fn
    {
        constexpr decltype(auto) operator()(const callocation<Alloc> auto& rng) //
            noexcept(noexcept(*allocation_data<T>(cpp_forward(rng))))
        {
            return *std::launder(allocation_data<T>(cpp_forward(rng)));
        }
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    static constexpr allocation_get_fn<Alloc, T> allocation_get{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    struct allocation_cget_fn
    {
        constexpr decltype(auto) operator()(const callocation<Alloc> auto& rng) //
            noexcept(noexcept(*allocation_cdata<T>(cpp_forward(rng))))
        {
            return *std::launder(allocation_cdata<T>(cpp_forward(rng)));
        }
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    static constexpr allocation_cget_fn<Alloc, T> allocation_cget{};

    template<typename Rng, typename Alloc>
    concept callocations_view = std::ranges::view<Rng> && requires(const Rng& crng) {
        requires std::ranges::input_range<decltype(crng)>;
        requires callocation<range_const_reference_t<decltype(crng)>, Alloc>;
    };

    template<typename Rng, typename Alloc>
    concept allocations_view = callocations_view<Rng, Alloc> && requires(const Rng& crng) {
        requires std::ranges::
            output_range<decltype(crng), std::ranges::range_rvalue_reference_t<decltype(crng)>>;
        requires allocation<std::ranges::range_value_t<decltype(crng)>, Alloc>;
    };

    template<allocator_req Allocator, callocations_view<Allocator> Allocations>
    struct const_source_allocations
    {
        std::reference_wrapper<const Allocator> allocator;
        Allocations allocations;
    };

    template<typename T, typename U>
    const_source_allocations(T&, U) -> const_source_allocations<T, U>;

    template<allocator_req Allocator, allocations_view<Allocator> Allocations>
    struct source_allocations
    {
        std::reference_wrapper<Allocator> allocator;
        Allocations allocations;

        constexpr operator const_source_allocations<Allocator, Allocations>() const noexcept
        {
            return {allocator, allocations};
        }
    };

    template<typename T, typename U>
    source_allocations(T&, U) -> source_allocations<T, U>;

    template<std::invocable GetAllocator, typename GetAllocations>
        requires allocator_req<std::invoke_result_t<GetAllocator>>
    struct [[nodiscard]] defer_allocations
    {
        GetAllocator get_allocator;
        GetAllocations get_allocations;

        template<typename Dst>
        static constexpr auto allocations_gettable =
            requires(std::invoke_result_t<GetAllocator> alloc) {
                requires allocations_view<Dst, decltype(alloc)>;
                requires std::invocable<GetAllocations, const Dst&, decltype(alloc)>;
            };

        template<typename Dst>
        static constexpr auto nothrow_allocations_gettable =
            requires(std::invoke_result_t<GetAllocator> alloc) {
                requires allocations_view<Dst, decltype(alloc)>;
                requires nothrow_invocable<GetAllocations, const Dst&, decltype(alloc)>;
            };
    };

    template<typename T, typename U>
    defer_allocations(T&&, U&&) -> defer_allocations<std::decay_t<T>, std::decay_t<U>>;
}