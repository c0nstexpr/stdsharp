#pragma once

#include "../allocator_traits.h"

namespace stdsharp::allocator_aware
{
    template<allocator_req>
    class allocation;

    namespace details
    {
        struct allocation_access
        {
            template<typename allocator_type>
            static constexpr allocation<allocator_type> make_allocation(
                allocator_type& alloc,
                const allocator_size_type<allocator_type> size,
                const allocator_cvp<allocator_type> hint = nullptr
            )
            {
                if(size == 0) [[unlikely]]
                    return {};

                return {allocator_traits<allocator_type>::allocate(alloc, size, hint), size};
            }

            template<typename allocator_type>
            static constexpr allocation<allocator_type> try_make_allocation(
                allocator_type& alloc,
                const allocator_size_type<allocator_type> size,
                const allocator_cvp<allocator_type> hint = nullptr
            ) noexcept
            {
                if(size == 0) [[unlikely]]
                    return {};

                return {allocator_traits<allocator_type>::try_allocate(alloc, size, hint), size};
            }
        };
    }

    template<allocator_req allocator_type>
    constexpr allocation<allocator_type> make_allocation(
        allocator_type& alloc,
        const allocator_size_type<allocator_type> size,
        const allocator_cvp<allocator_type> hint = nullptr
    )
    {
        return details::allocation_access::make_allocation(alloc, size, hint);
    }

    template<allocator_req allocator_type>
    constexpr allocation<allocator_type> try_make_allocation(
        allocator_type& alloc,
        const allocator_size_type<allocator_type> size,
        const allocator_cvp<allocator_type> hint = nullptr
    ) noexcept
    {
        return details::allocation_access::try_make_allocation(alloc, size, hint);
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
        friend details::allocation_access;

        static auto constexpr always_equal_v = traits::always_equal_v;
        static auto constexpr propagate_on_swap_v = traits::propagate_on_swap_v;

        pointer ptr_ = nullptr;
        size_type size_ = 0;

    protected:
        constexpr allocation(pointer ptr, const size_type size) noexcept: ptr_(ptr), size_(size) {}

    public:
        allocation() = default;

        [[nodiscard]] constexpr auto begin() const noexcept { return ptr_; }

        [[nodiscard]] constexpr auto end() const noexcept { return ptr_ + size_; }

        [[nodiscard]] constexpr const_pointer cbegin() const noexcept { return begin(); }

        [[nodiscard]] constexpr const_pointer cend() const noexcept { return end(); }

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        [[nodiscard]] constexpr auto empty() const noexcept { return ptr_ != nullptr; }

        constexpr operator bool() const noexcept { return empty(); }

        constexpr void allocate(
            allocator_type& alloc,
            const size_type size,
            const allocator_cvp<allocator_type> hint = nullptr
        )
        {
            if(size_ >= size) return;

            if(ptr_ != nullptr) deallocate(alloc);

            *this = make_allocation(alloc, size, hint);
        }

        constexpr void deallocate(allocator_type& alloc) noexcept
        {
            traits::deallocate(alloc, ptr_, size_);
            *this = {};
        }

    private:
        constexpr bool vaildate_alloc(const allocator_type& left, const allocator_type& right) noexcept
        {
            if constexpr(always_equal_v) return true;
            else return left == right;
        }

    public:
        constexpr void swap(
            allocator_type& src_alloc,
            allocator_type& dst_alloc,
            allocation& dst_allocation
        ) noexcept
        {
            Expects((vaildate_alloc(dst_alloc, src_alloc)));
            std::swap(dst_allocation, *this);
            if constexpr(propagate_on_swap_v) std::ranges::swap(dst_alloc, src_alloc);
        }
    };
}