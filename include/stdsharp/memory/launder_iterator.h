#pragma once

#include <gsl/pointers>
#include <new>

#include "../default_operator.h"

namespace stdsharp
{
    template<typename>
    struct launder_iterator;

    template<typename T>
    class launder_iterator<T*> :
        gsl::not_null<T*>,
        default_increase_and_decrease<launder_iterator<T*>>
    {
        using m_base = gsl::not_null<T*>;

    public:
        constexpr launder_iterator(T* ptr) noexcept: m_base(ptr) {}

        launder_iterator(nullptr_t) = delete;

        launder_iterator(launder_iterator&&) noexcept = default;
        launder_iterator& operator=(const launder_iterator&) noexcept = default;
        launder_iterator& operator=(launder_iterator&&) noexcept = default;
        launder_iterator(const launder_iterator&) noexcept = default;
        ~launder_iterator() noexcept = default;

        [[nodiscard]] constexpr decltype(auto) operator*() const noexcept
        {
            return *std::launder(this->get());
        }

        [[nodiscard]] constexpr auto operator->() const noexcept
        {
            return std::launder(this->get());
        }

        bool operator==(const launder_iterator& other) const = default;

        constexpr void operator++() noexcept
        {
            auto ptr = this->get();
            *this = ++ptr;
        }

        constexpr void operator--() noexcept
        {
            auto ptr = this->get();
            *this = --ptr;
        }
    };

    template<typename T>
    launder_iterator(T*) -> launder_iterator<T*>;
}