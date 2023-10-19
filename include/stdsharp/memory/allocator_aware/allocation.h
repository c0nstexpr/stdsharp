#pragma once

#include "../allocator_traits.h"
#include "../pointer_traits.h"

namespace stdsharp
{
    template<allocator_req Alloc>
    class [[nodiscard]] allocation
    {
    public:
        using allocator_type = Alloc;
        using pointer = allocator_pointer<allocator_type>;
        using size_type = allocator_size_type<allocator_type>;
        using value_type = allocator_value_type<allocator_type>;
        using const_pointer = allocator_const_pointer<allocator_type>;

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

        template<typename T = value_type>
        [[nodiscard]] constexpr decltype(auto) data() const noexcept
        {
            if constexpr(std::same_as<T, value_type>) return begin();
            else return pointer_cast<T>(begin());
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr decltype(auto) cdata() const noexcept
        {
            if constexpr(std::same_as<T, value_type>) return cbegin();
            else return pointer_cast<T>(cbegin());
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr T& get() const noexcept
        {
            Expects(!empty());
            return *data<T>();
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr const T& cget() const noexcept
        {
            Expects(!empty());
            return *cdata<T>();
        }

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        [[nodiscard]] constexpr auto empty() const noexcept { return ptr_ == nullptr; }
    };

    template<typename Allocation>
    class [[nodiscard]] callocation : Allocation
    {
        using allocation = Allocation;
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
        [[nodiscard]] constexpr auto data() const noexcept
        {
            return cdata<T>();
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr T& get() const noexcept
        {
            return allocation::template get<T>();
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr const T& cget() const noexcept
        {
            return allocation::template cget<T>();
        }
    };
}