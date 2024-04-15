#pragma once

#include "../default_operator.h"
#include "../iterator/basic_iterator.h"

#include <new>

namespace stdsharp
{
    template<typename T> // NOLINTBEGIN(*-pointer-arithmetic)
    class launder_iterator : public basic_iterator, public std::random_access_iterator_tag
    {
        T* ptr_;

        constexpr void not_null() const noexcept { assert_not_null(ptr_); }

        [[nodiscard]] constexpr auto& data() noexcept { return ptr_; }

        friend basic_iterator;

    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        launder_iterator() = default;

        constexpr launder_iterator(T* const ptr) noexcept: ptr_(ptr) { not_null(); }

        launder_iterator(nullptr_t) = delete;

        launder_iterator(launder_iterator&&) noexcept = default;
        launder_iterator& operator=(const launder_iterator&) noexcept = default;
        launder_iterator& operator=(launder_iterator&&) noexcept = default;
        launder_iterator(const launder_iterator&) noexcept = default;
        ~launder_iterator() noexcept = default;

        [[nodiscard]] constexpr decltype(auto) operator[](const difference_type diff) const noexcept
        {
            not_null();
            return *std::launder(ptr_ + diff);
        }

        using default_operator::subscript::operator[];
        using default_operator::arithmetic::operator-;
    }; // NOLINTEND(*-pointer-arithmetic)

    template<typename T>
    launder_iterator(T*) -> launder_iterator<T>;
}