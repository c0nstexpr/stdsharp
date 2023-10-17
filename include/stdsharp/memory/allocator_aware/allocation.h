#pragma once

#include "../allocator_traits.h"
#include "../pointer_traits.h"

namespace stdsharp::allocator_aware
{
    template<allocator_req>
    class allocation;

    template<allocator_req allocator_type>
    constexpr allocation<allocator_type> make_allocation(
        allocator_type& alloc,
        const allocator_size_type<allocator_type> size,
        const allocator_cvp<allocator_type> hint = nullptr
    )
    {
        if(size == 0) [[unlikely]]
            return {};
        return {allocator_traits<allocator_type>::allocate(alloc, size, hint), size};
    }

    template<allocator_req allocator_type>
    constexpr allocation<allocator_type> try_make_allocation(
        allocator_type& alloc,
        const allocator_size_type<allocator_type> size,
        const allocator_cvp<allocator_type> hint = nullptr
    ) noexcept
    {
        if(size == 0) [[unlikely]]
            return {};
        return {allocator_traits<allocator_type>::try_allocate(alloc, size, hint), size};
    }

    template<allocator_req Allocator>
    class [[nodiscard]] allocation
    {
    public:
        using allocator_type = Allocator;
        using traits = allocator_traits<allocator_type>;
        using pointer = traits::pointer;
        using const_pointer = traits::const_pointer;
        using size_type = traits::size_type;

    private:
        pointer ptr_ = nullptr;
        size_type size_ = 0;

    public:
        allocation() = default;

        constexpr allocation(pointer ptr, const size_type size) noexcept: ptr_(ptr), size_(size) {}

        [[nodiscard]] constexpr auto begin() const noexcept { return ptr_; }

        [[nodiscard]] constexpr auto end() const noexcept { return ptr_ + size_; }

        [[nodiscard]] constexpr const_pointer cbegin() const noexcept { return begin(); }

        [[nodiscard]] constexpr const_pointer cend() const noexcept { return end(); }

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        [[nodiscard]] constexpr auto empty() const noexcept { return ptr_ == nullptr; }

        constexpr operator bool() const noexcept { return !empty(); }

        constexpr void allocate(
            allocator_type& alloc,
            const size_type size,
            const allocator_cvp<allocator_type> hint = nullptr
        )
        {
            Expects(empty());
            *this = make_allocation(alloc, size, hint);
        }

        constexpr void deallocate(allocator_type& alloc) noexcept
        {
            traits::deallocate(alloc, ptr_, size_);
            *this = {};
        }

        template<typename T, typename... Args>
            requires(traits::template constructible_from<T, Args...>)
        constexpr T& construct(allocator_type& alloc, Args&&... args) //
            noexcept(traits::template nothrow_constructible_from<T, Args...>)
        {
            Expects(size() >= sizeof(T));

            traits::construct(alloc, ptr_, cpp_forward(args)...);

            return *pointer_cast<T>(begin());
        }

        template<typename T>
            requires std::is_destructible_v<T>
        constexpr void destroy(allocator_type& alloc) noexcept(std::destructible<T>)
        {
            if(empty()) return;
            traits::destroy(alloc, pointer_cast<T>(begin()));
        }
    };
}