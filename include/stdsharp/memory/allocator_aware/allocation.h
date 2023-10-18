#pragma once

#include "../allocator_traits.h"
#include "../pointer_traits.h"

namespace stdsharp
{
    template<allocator_req Alloc>
    class [[nodiscard]] allocation
    {
    public:
        using pointer = allocator_pointer<Alloc>;
        using size_type = allocator_size_type<Alloc>;
        using value_type = allocator_value_type<Alloc>;
        using const_pointer = allocator_const_pointer<Alloc>;

    private:
        pointer ptr_ = nullptr;
        size_type size_ = 0;

    public:
        allocation() = default;

        constexpr allocation(const pointer ptr, const size_type size) noexcept:
            ptr_(ptr), size_(size)
        {
        }

        [[nodiscard]] constexpr auto begin() const noexcept { return ptr_; }

        [[nodiscard]] constexpr auto end() const noexcept { return ptr_ + size_; }

        [[nodiscard]] constexpr const_pointer cbegin() const noexcept { return begin(); }

        [[nodiscard]] constexpr const_pointer cend() const noexcept { return end(); }

        [[nodiscard]] constexpr pointer data() const noexcept { return begin(); }

        [[nodiscard]] constexpr const_pointer cdata() const noexcept { return cbegin(); }

        template<typename T>
        [[nodiscard]] constexpr auto data() const noexcept
        {
            return pointer_cast<T>(data());
        }

        template<typename T>
        [[nodiscard]] constexpr auto cdata() const noexcept
        {
            return pointer_cast<T>(cdata());
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr decltype(auto) get() const noexcept
        {
            Expects(!empty());
            return *data<T>();
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr decltype(auto) cget() const noexcept
        {
            Expects(!empty());
            return *cdata<T>();
        }

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        [[nodiscard]] constexpr auto empty() const noexcept { return ptr_ == nullptr; }
    };

    template<allocator_req Alloc>
    class [[nodiscard]] callocation : allocation<Alloc>
    {
        using allocation = allocation<Alloc>;
        using typename allocation::value_type;
        using allocation::allocation;
        using allocation::cbegin;
        using allocation::cend;
        using allocation::size;
        using allocation::empty;

        template<typename T = value_type>
        [[nodiscard]] constexpr auto cdata() const noexcept
        {
            return allocation::template cdata<T>();
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr decltype(auto) cget() const noexcept
        {
            return allocation::template cget<T>();
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr auto data() const noexcept
        {
            return cdata<T>();
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr decltype(auto) get() const noexcept
        {
            return cget<T>();
        }
    };
}