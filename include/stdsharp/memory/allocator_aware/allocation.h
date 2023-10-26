#pragma once

#include "../allocator_traits.h"
#include "../pointer_traits.h"

namespace stdsharp::allocator_aware
{
    template<typename T, typename Alloc>
    concept allocation = std::ranges::sized_range<T> && requires(const T t) {
        {
            std::ranges::cdata(t)
        } noexcept -> std::same_as<allocator_const_pointer<Alloc>>;
        {
            std::ranges::data(t)
        } noexcept -> std::same_as<allocator_pointer<Alloc>>;
        {
            std::ranges::empty(t)
        } noexcept;
    };

    template<typename T, typename Alloc>
    concept allocation = std::ranges::sized_range<T> && requires(const T t) {
        {
            std::ranges::cdata(t)
        } noexcept -> std::same_as<allocator_const_pointer<Alloc>>;
        {
            std::ranges::data(t)
        } noexcept -> std::same_as<allocator_pointer<Alloc>>;
        {
            std::ranges::empty(t)
        } noexcept;
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    static constexpr decltype(auto) data =
        []() noexcept
    {
        if constexpr(std::same_as<T, value_type>) return begin();
        else return pointer_cast<T>(begin());
    }

    template<typename T = value_type>
    [[nodiscard]] constexpr decltype(auto) cdata() const noexcept
    {
        if constexpr(std::same_as<T, value_type>) return cbegin();
        else return pointer_cast<T>(cbegin());
    }

    template<typename T = value_type>
    [[nodiscard]] constexpr T& get() const noexcept
    {
        Expects(!empty());
        return *std::launder(data<T>());
    }

    template<typename T = value_type>
    [[nodiscard]] constexpr const T& cget() const noexcept
    {
        Expects(!empty());
        return *std::launder(cdata<T>());
    }

    template<typename Alloc, typename Rng>
    concept callocations_view = std::ranges::input_range<Rng> && //
        std::ranges::view<Rng> && //
        callocation<std::ranges::range_value_t<Rng>>;

    template<typename Rng, typename Alloc>
    concept allocations_view = std::ranges::input_range<Rng> && //
        std::ranges::view<Rng> && //
        callocation<std::ranges::range_value_t<Rng>> &&
        std::ranges::output_range<Rng, allocation<Alloc>>;

    template<allocator_req Allocator, allocations_view<Allocator> Allocations>
    struct target_allocations
    {
        std::reference_wrapper<Allocator> allocator;
        Allocations allocations;
    };

    template<typename T, typename U>
    target_allocations(T&, U) -> target_allocations<T, U>;

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

        constexpr auto to_const() const noexcept
        {
            return const_source_allocations{
                allocator.get(), // TODO: replace with std::views::as_const
                allocations | views::cast<callocation<Allocator>>
            };
        }
    };

    template<typename T, typename U>
    source_allocations(T&, U) -> source_allocations<T, U>;

    template<allocator_req Allocator, std::invocable<Allocator&> DeferAllocations>
    struct ctor_input_allocations
    {
        Allocator allocator;
        DeferAllocations deferred_allocations;
    };

    template<typename T, typename U>
    ctor_input_allocations(T, U) -> ctor_input_allocations<T, U>;

    template<typename T, typename UDefer>
    ctor_input_allocations(T, UDefer) -> ctor_input_allocations<T, std::invoke_result<UDefer, T&>>;
}