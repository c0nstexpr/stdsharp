#pragma once

#include "allocator_traits.h"
#include "concepts.h"

namespace stdsharp
{
    template<typename T, typename Alloc>
    concept callocation = requires(const T& t) {
        requires std::ranges::sized_range<T>;
        {
            std::ranges::cbegin(t)
        } noexcept -> std::same_as<allocator_const_pointer<Alloc>>;
        {
            std::ranges::size(t)
        } noexcept -> std::same_as<allocator_size_type<Alloc>>;
    };
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
        template<typename T>
        static constexpr auto assignable_from_shadow = nothrow_assignable_from<T&, shadow_type>;
    };
}

namespace stdsharp
{
    template<typename T, typename Alloc>
    concept allocation = requires(
        const T& ct,
        std::decay_t<T> decay_t,
        allocator_pointer<Alloc> p,
        allocator_size_type<Alloc> size
    ) {
        requires callocation<T, Alloc>;

        {
            std::ranges::begin(ct)
        } noexcept -> std::same_as<decltype(p)>;

        requires nothrow_default_initializable<decltype(decay_t)>;
        requires nothrow_constructible_from<decltype(decay_t), decltype(p), decltype(size)>;
        requires nothrow_movable<decltype(decay_t)>;
        requires details::allocation_concept<Alloc>::template assignable_from_shadow<
            decltype(decay_t)>;
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
    inline constexpr allocation_data_fn<Alloc, T> allocation_data{};

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
    inline constexpr allocation_cdata_fn<Alloc, T> allocation_cdata{};

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
    inline constexpr allocation_get_fn<Alloc, T> allocation_get{};

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
    inline constexpr allocation_cget_fn<Alloc, T> allocation_cget{};

    template<allocator_req Alloc>
    struct make_callocation_fn
    {
    private:
        class callocation_t
        {
            allocator_const_pointer<Alloc> p_{};
            allocator_size_type<Alloc> size_{};

        public:
            constexpr callocation_t(
                const allocator_const_pointer<Alloc> p,
                const allocator_size_type<Alloc> size
            ) noexcept:
                p_(p), size_(size)
            {
            }

            [[nodiscard]] constexpr auto begin() const noexcept { return p_; }

            [[nodiscard]] constexpr auto end() const noexcept { return p_ + size_; }

            [[nodiscard]] constexpr auto size() const noexcept { return size_; }
        };

    public:
        constexpr callocation_t operator()(const callocation<Alloc> auto rng) const noexcept
        {
            return {std::ranges::cbegin(rng), std::ranges::size(rng)};
        }
    };

    template<allocator_req Alloc>
    inline constexpr make_callocation_fn<Alloc> make_callocation{};

    template<typename Rng, typename Alloc>
    concept callocations_view = std::ranges::borrowed_range<Rng> && //
        nothrow_forward_range<Rng> && //
        callocation<range_const_reference_t<Rng>, Alloc>;

    template<typename Rng, typename Alloc>
    concept allocations_view = callocations_view<Rng, Alloc> && //
        range_movable<Rng, Rng> && //
        allocation<std::ranges::range_value_t<Rng>, Alloc>;

    namespace views
    {
        template<allocator_req Alloc>
        inline constexpr auto callocations = std::views::transform(make_callocation<Alloc>);

        template<allocator_req Alloc, callocations_view<Alloc> Allocations>
        using callocations_t = decltype(callocations<Alloc>(std::declval<Allocations>()));
    }
}