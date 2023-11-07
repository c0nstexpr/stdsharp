#pragma once

#include "allocator_traits.h"
#include "concepts.h"
#include "../functional/compose.h"

namespace stdsharp
{
    template<allocator_req Alloc>
    struct callocation_result
    {
        allocator_const_pointer<Alloc> p{};
        allocator_size_type<Alloc> diff{};

        [[nodiscard]] constexpr auto begin() const noexcept { return p; }

        [[nodiscard]] constexpr auto end() const noexcept { return p + diff; }

        [[nodiscard]] constexpr auto size() const noexcept { return diff; }
    };

    template<allocator_req Alloc>
    struct allocation_result
    {
        allocator_pointer<Alloc> p{};
        allocator_size_type<Alloc> diff{};

        [[nodiscard]] constexpr auto begin() const noexcept { return p; }

        [[nodiscard]] constexpr auto end() const noexcept { return p + diff; }

        [[nodiscard]] constexpr auto size() const noexcept { return diff; }

        constexpr auto& operator=(const auto& t) noexcept
            requires requires(
                const decltype(std::ranges::cbegin(t))& begin,
                const decltype(std::ranges::size(t))& size
            ) {
                {
                    p = begin
                } noexcept;
                {
                    diff = size
                } noexcept;
            }
        {
            p = std::ranges::cbegin(t);
            diff = std::ranges::size(t);
            return *this;
        }

        constexpr operator callocation_result<Alloc>() const noexcept { return {p, diff}; }
    };

    template<allocator_req Alloc>
    inline constexpr allocation_result<Alloc> empty_allocation_result{};
}

namespace stdsharp::details
{
    template<typename Alloc>
    struct allocation_concept
    {
    private:
        struct shadow_type
        {
            [[nodiscard]] constexpr auto begin() const noexcept
            {
                return allocator_pointer<Alloc>{nullptr};
            }

            [[nodiscard]] constexpr auto end() const noexcept
            {
                return allocator_pointer<Alloc>{nullptr};
            }

            [[nodiscard]] constexpr auto size() const noexcept
            {
                return allocator_size_type<Alloc>{0};
            }

            constexpr auto& operator=(const auto& /*unused*/) noexcept { return *this; }
        };

    public:
        template<typename T, typename LRef = std::add_lvalue_reference_t<std::decay_t<T>>>
        static constexpr auto assignable_from_shadow = nothrow_assignable_from<LRef, shadow_type> &&
            nothrow_assignable_from<LRef, allocation_result<Alloc>>;
    };

    template<typename T, typename Alloc>
    concept allocation_common = requires(const T& t) {
        requires nothrow_copyable<std::decay_t<T>>;
        requires std::ranges::sized_range<T>;
        {
            std::ranges::size(t)
        } noexcept -> std::same_as<allocator_size_type<Alloc>>;
    };
}

namespace stdsharp
{
    template<typename T, typename Alloc>
    concept callocation = requires(const T& t) {
        requires details::allocation_common<T, Alloc>;
        {
            std::ranges::cbegin(t)
        } noexcept -> std::same_as<allocator_const_pointer<Alloc>>;
    };

    template<typename T, typename Alloc>
    concept allocation = requires(const T& t) {
        requires details::allocation_common<T, Alloc>;
        {
            std::ranges::cbegin(t)
        } noexcept -> std::same_as<allocator_pointer<Alloc>>;
        requires details::allocation_concept<Alloc>:: //
            template assignable_from_shadow<T>;
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    struct allocation_data_fn
    {
        template<typename U>
        constexpr decltype(auto) operator()(const U & rng) const noexcept
            requires(callocation<U, Alloc> || allocation<U, Alloc>)
        {
            return pointer_cast<T>(std::ranges::cbegin(rng));
        }
    };

    template<allocator_req Alloc>
    struct allocation_data_fn<Alloc, typename Alloc::value_type>
    {
        template<typename U>
        constexpr decltype(auto) operator()(const U & rng) const noexcept
            requires(callocation<U, Alloc> || allocation<U, Alloc>)
        {
            return std::ranges::cbegin(rng);
        }
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    inline constexpr allocation_data_fn<Alloc, T> allocation_data{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    using allocation_cdata_fn =
        composed<allocation_data_fn<Alloc, T>, cast_to_fn<allocator_const_pointer<Alloc>>>;

    template<allocator_req Alloc, typename T = Alloc::value_type>
    inline constexpr allocation_cdata_fn<Alloc, T> allocation_cdata{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    struct allocation_get_fn
    {
        template<typename U>
        constexpr decltype(auto) operator()(const U & rng) const
            noexcept(noexcept(*allocation_data<Alloc, T>(rng)))
            requires std::invocable<allocation_data_fn<Alloc, T>, const U&>
        {
            return *std::launder(allocation_data<Alloc, T>(rng));
        }
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    inline constexpr allocation_get_fn<Alloc, T> allocation_get{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    using allocation_cget_fn = allocation_get_fn<Alloc, const T>;

    template<allocator_req Alloc, typename T = Alloc::value_type>
    inline constexpr allocation_cget_fn<Alloc, T> allocation_cget{};

    template<typename Rng, typename Alloc>
    concept callocations_view = std::ranges::borrowed_range<Rng> && //
        constant_range<Rng> && //
        nothrow_forward_range<Rng> && //
        callocation<range_const_reference_t<Rng>, Alloc>;

    template<typename Rng, typename Alloc>
    concept allocations_view = std::ranges::borrowed_range<Rng> && //
        range_movable<Rng, Rng> && //
        nothrow_forward_range<Rng> && //
        allocation<std::ranges::range_value_t<Rng>, Alloc>;
}