#pragma once

#include "../iterator/basic_iterator.h"

#include <bitset>
#include <iterator>
#include <ranges>

namespace stdsharp::details
{
    template<std::size_t N, bool IsConst>
    class bitset_iterator_base : public basic_iterator<>
    {
    public:
        using bitset = std::bitset<N>;

    protected:
        using maybe_const = std::conditional_t<IsConst, const bitset, bitset>;

        maybe_const* set_{};
        std::size_t i_{};

        friend basic_iterator<>;

        [[nodiscard]] constexpr auto& data() noexcept { return i_; }

        [[nodiscard]] constexpr const auto& data() const noexcept { return i_; }

    public:
        bitset_iterator_base() = default;

        constexpr bitset_iterator_base(maybe_const& set, const std::size_t index):
            set_(&set), i_(index)
        {
        }

        [[nodiscard]] constexpr decltype(auto) operator*() const { return (*set_)[i_]; }
    };

    template<std::size_t N, bool IsConst, typename Base = bitset_iterator_base<N, IsConst>>
    struct bitset_iterator : Base, iterator_value_type_traits<Base>
    {
        [[nodiscard]] constexpr decltype(auto) operator[](const std::iter_difference_t<Base> index
        ) const
        {
            return (*this->set_)[this->i_ + index];
        }
    };
}

namespace stdsharp
{
    template<std::size_t N>
    struct bitset_iterator : details::bitset_iterator<N, false>
    {
    };

    template<std::size_t N>
    struct bitset_const_iterator : details::bitset_iterator<N, true>
    {
    };

    template<std::size_t N>
    bitset_iterator(std::bitset<N>&, auto) -> bitset_iterator<N>;

    template<std::size_t N>
    bitset_const_iterator(const std::bitset<N>&, auto) -> bitset_const_iterator<N>;

    inline constexpr struct bitset_crng_fn
    {
        template<std::size_t N>
        [[nodiscard]] constexpr auto operator()(const std::bitset<N>& set) const
        {
            return std::ranges::
                subrange{bitset_const_iterator{set, 0}, bitset_const_iterator{set, N}};
        }
    } bitset_crng{};

    inline constexpr struct bitset_rng_fn : bitset_crng_fn
    {
        template<std::size_t N>
        [[nodiscard]] constexpr auto operator()(std::bitset<N>& set) const
        {
            return std::ranges::subrange{bitset_iterator{set, 0}, bitset_iterator{set, N}};
        }
    } bitset_rng{};
}
