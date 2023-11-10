#pragma once

#include <gsl/pointers>
#include <new>

#include "../default_operator.h"
#include "../cassert/cassert.h"

namespace stdsharp
{
    template<typename T>
    class launder_iterator :
        default_increase_and_decrease<launder_iterator<T*>>,
        default_arithmetic_operation<launder_iterator<T*>>,
        public std::random_access_iterator_tag
    {
        T* ptr_;

    public:
        constexpr launder_iterator(T* const ptr) noexcept: ptr_((assert_not_null(ptr), ptr)) {}

        launder_iterator(nullptr_t) = delete;

        launder_iterator(launder_iterator&&) noexcept = default;
        launder_iterator& operator=(const launder_iterator&) noexcept = default;
        launder_iterator& operator=(launder_iterator&&) noexcept = default;
        launder_iterator(const launder_iterator&) noexcept = default;
        ~launder_iterator() noexcept = default;

        [[nodiscard]] constexpr decltype(auto) operator*() const noexcept
        {
            return *std::launder(ptr_);
        }

        [[nodiscard]] constexpr auto operator->() const noexcept { return std::launder(ptr_); }

        bool operator==(const launder_iterator& other) const = default;

        constexpr void operator++() noexcept { ++ptr_; }

        constexpr void operator--() noexcept { --ptr_; }

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

        constexpr std::ptrdiff_t operator-(const launder_iterator& other) const noexcept
        {
            return ptr_ - other.ptr_;
        }

        constexpr decltype(auto) operator[](const std::ptrdiff_t diff) noexcept { return *(ptr_ + diff); }

        constexpr decltype(auto) operator[](const std::ptrdiff_t diff) const noexcept { return *(ptr_ + diff); }

        friend auto operator<=>(const launder_iterator&, const launder_iterator&) = default;
    };

    template<typename T>
    launder_iterator(T*) -> launder_iterator<T>;
}