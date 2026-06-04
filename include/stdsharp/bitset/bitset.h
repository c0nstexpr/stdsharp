#pragma once
#include "../cassert/cassert.h"
#include "../cstdint/cstdint.h"
#include "../iterator/basic_iterator.h"
#include "../utility/forward_like.h"

#include <bitset>
#include <ranges>

namespace stdsharp::details
{
    template<typename Bitset>
    class bitset_iterator : public basic_iterator<std::decay_t<decltype(Bitset{}[0])>, ssize_t>
    {
        using m_base = basic_iterator<//
            typename bitset_iterator::value_type,
            typename bitset_iterator::difference_type>;

    public:
        using bitset = Bitset;

    private:
        bitset* set_{};
        std::size_t i_{};

        template<typename T>
        static constexpr auto self_cast = forward_like<T, bitset_iterator>;

        constexpr void validate_index(ssize_t n = 0) const noexcept
        {
            assert_less(i_ + n, set_->size());
        }

    public:
        bitset_iterator() = default;

        constexpr bitset_iterator(bitset& set, const std::size_t index): set_(&set), i_(index) {}

        [[nodiscard]] constexpr decltype(auto) operator*() const { return (*set_)[i_]; }

        template<non_const Self>
        constexpr auto& operator++(this Self& self) noexcept
        {
            auto& this_ = self_cast<Self>(self);
            this_.validate_index(1);
            ++this_.i_;
            return self;
        }

        template<non_const Self>
        constexpr auto& operator--(this Self& self) noexcept
        {
            auto& this_ = self_cast<Self>(self);
            this_.validate_index(1);
            --this_.i_;
            return self;
        }

        template<non_const Self>
        constexpr auto& operator+=(this Self& self, const ssize_t n) noexcept
        {
            auto& this_ = self_cast<Self>(self);
            this_.validate_index(n);
            this_.i_ += n;
            return self;
        }

        [[nodiscard]] constexpr auto operator-(const bitset_iterator& other) const noexcept
        {
            assert_equal(this->set_, other.set_);
            return static_cast<ssize_t>(i_) - static_cast<ssize_t>(other.i_);
        }

        [[nodiscard]] constexpr decltype(auto) operator[](const ssize_t index) const
        {
            validate_index(index);
            return (*set_)[i_ + index];
        }

        [[nodiscard]] constexpr bool operator==(const bitset_iterator& other) const noexcept
        {
            assert_equal(set_, other.set_);
            return i_ == other.i_;
        }

        [[nodiscard]] constexpr auto operator<=>(const bitset_iterator& other) const noexcept
        {
            assert_equal(set_, other.set_);
            return i_ <=> other.i_;
        }
    };
}

namespace stdsharp
{
    template<std::size_t N>
    struct bitset_iterator : details::bitset_iterator<std::bitset<N>>
    {
    private:
        using m_base = details::bitset_iterator<std::bitset<N>>;

    public:
        using m_base::m_base;
    };

    template<std::size_t N>
    bitset_iterator(std::bitset<N>&, auto) -> bitset_iterator<N>;

    template<std::size_t N>
    struct bitset_const_iterator : details::bitset_iterator<const std::bitset<N>>
    {
    private:
        using m_base = details::bitset_iterator<const std::bitset<N>>;

    public:
        using m_base::m_base;
    };

    template<std::size_t N>
    bitset_const_iterator(const std::bitset<N>&, auto) -> bitset_const_iterator<N>;

    inline constexpr struct bitset_crng_fn
    {
        template<std::size_t N>
        [[nodiscard]] static constexpr auto operator()(const std::bitset<N>& set)
        {
            return std::ranges::
                subrange{bitset_const_iterator{set, 0}, bitset_const_iterator{set, N}};
        }
    } bitset_crng{};

    inline constexpr struct bitset_rng_fn : bitset_crng_fn
    {
        using bitset_crng_fn::operator();

        template<std::size_t N>
        [[nodiscard]] static constexpr auto operator()(std::bitset<N>& set)
        {
            return std::ranges::subrange{bitset_iterator{set, 0}, bitset_iterator{set, N}};
        }
    } bitset_rng{};
}
