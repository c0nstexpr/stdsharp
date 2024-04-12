#pragma once

#include "../cassert/cassert.h"
#include "../default_operator.h"

#include <new>

namespace stdsharp
{
    template<typename T> // NOLINTBEGIN(*-pointer-arithmetic)
    class launder_iterator :
        public default_operator::arithmetic,
        public default_operator::arrow,
        public std::random_access_iterator_tag
    {
        T* ptr_;

    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        constexpr launder_iterator(T* const ptr) noexcept: ptr_((assert_not_null(ptr), ptr)) {}

        launder_iterator(nullptr_t) = delete;

        launder_iterator(launder_iterator&&) noexcept = default;
        launder_iterator& operator=(const launder_iterator&) noexcept = default;
        launder_iterator& operator=(launder_iterator&&) noexcept = default;
        launder_iterator(const launder_iterator&) noexcept = default;
        ~launder_iterator() noexcept = default;

        [[nodiscard]] constexpr auto ptr() const noexcept { return ptr_; }

        [[nodiscard]] constexpr auto data() const noexcept { return std::launder(ptr_); }

        [[nodiscard]] constexpr auto operator<=>(const launder_iterator other) const noexcept
        {
            return ptr_ <=> other.ptr_;
        }

        [[nodiscard]] constexpr bool operator==(const launder_iterator other) const noexcept
        {
            return ptr_ == other.ptr_;
        }

        constexpr launder_iterator& operator+=(const difference_type diff) noexcept
        {
            ptr_ += diff;
            return *this;
        }

        constexpr launder_iterator& operator-=(const difference_type diff) noexcept
        {
            ptr_ -= diff;
            return *this;
        }

        [[nodiscard]] constexpr difference_type operator-(const launder_iterator& other) //
            const noexcept
        {
            return ptr_ - other.ptr_;
        }

        [[nodiscard]] constexpr decltype(auto) operator*() const noexcept { return *data(); }

        [[nodiscard]] constexpr decltype(auto) operator[](const difference_type diff) const noexcept
        {
            return *std::launder(ptr_ + diff);
        }
    }; // NOLINTEND(*-pointer-arithmetic)

    template<typename T>
    launder_iterator(T*) -> launder_iterator<T>;
}