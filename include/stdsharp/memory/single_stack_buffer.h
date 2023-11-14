#pragma once

#include "allocator_traits.h"
#include "../cstdint/cstdint.h"

namespace stdsharp
{
    inline constexpr auto default_soo_size = sizeof(std::max_align_t);

    template<std::size_t Size = default_soo_size>
    class single_stack_buffer
    {
    public:
        static constexpr auto size = Size;

        single_stack_buffer() = default;
        single_stack_buffer(const single_stack_buffer&) = delete;
        single_stack_buffer(single_stack_buffer&&) = delete;
        single_stack_buffer& operator=(const single_stack_buffer&) = delete;
        single_stack_buffer& operator=(single_stack_buffer&&) = delete;
        ~single_stack_buffer() = default;

        [[nodiscard]] constexpr void* try_allocate(const std::size_t required_size) noexcept
        {
            if(required_size > size) return nullptr;
            allocate_size_ = required_size;
            return data();
        }

        [[nodiscard]] constexpr auto allocate(const std::size_t required_size)
        {
            const auto ptr = try_allocate(required_size);

            return ptr == nullptr ? throw std::bad_alloc{} : ptr;
        }

        constexpr void deallocate(
            void* const p,
            const std::size_t required_size //
        ) noexcept
        {
            Expects(contains(p));
            Expects(allocate_size_ == required_size);
            allocate_size_ = 0;
        }

        [[nodiscard]] constexpr auto contains(const void* const in_ptr) noexcept
        {
            return in_ptr == data();
        }

        [[nodiscard]] constexpr void* data() const noexcept { return auto_cast(storage_.data()); }

        [[nodiscard]] constexpr bool operator==(const single_stack_buffer& other) const noexcept
        {
            return this == &other;
        }

    private:
        [[nodiscard]] constexpr void* data() noexcept { return auto_cast(storage_.data()); }

        std::size_t allocate_size_ = 0;
        alignas(std::max_align_t) std::array<byte, size> storage_{};
    };
}
