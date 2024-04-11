#pragma once

#include "../cassert/cassert.h"
#include "../default_operator.h"

#include <new>

namespace stdsharp
{
    template<typename T> // NOLINTBEGIN(*-pointer-arithmetic)
    class launder_iterator : default_operator::arithmetic, public std::random_access_iterator_tag
    {
        T* ptr_;

    public:
        using value_type = T;

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

        constexpr launder_iterator& operator+=(const std::ptrdiff_t diff) noexcept
        {
            ptr_ += diff;
            return *this;
        }

        constexpr launder_iterator& operator-=(const std::ptrdiff_t diff) noexcept
        {
            ptr_ -= diff;
            return *this;
        }

        [[nodiscard]] constexpr std::ptrdiff_t operator-(const launder_iterator& other) //
            const noexcept
        {
            return ptr_ - other.ptr_;
        }

        [[nodiscard]] constexpr auto operator->() const noexcept { return data(); }

        [[nodiscard]] constexpr decltype(auto) operator*() const noexcept { return *data(); }

        [[nodiscard]] constexpr decltype(auto) operator[](const std::ptrdiff_t diff) const noexcept
        {
            return *std::launder(ptr_ + diff);
        }
    }; // NOLINTEND(*-pointer-arithmetic)

    template<typename T>
    launder_iterator(T*) -> launder_iterator<T>;
}