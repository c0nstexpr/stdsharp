#pragma once

#include "aligned.h"
#include "allocator_traits.h"

#include "../compilation_config_in.h"

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

        [[nodiscard]] constexpr void*
            allocate(const std::size_t s, const std::size_t alignment = max_alignment_v) noexcept
        {
            if(s > size || p_ != nullptr) return nullptr;

            if(std::is_constant_evaluated())
            {
                Expects(alignment <= max_alignment_v);
                p_ = buffer_.data();
            }
            else if(const auto span = align(alignment, s, std::span{buffer_}); !span.empty())
                p_ = span.data();

            return p_;
        }

        constexpr void deallocate(void* const p) noexcept
        {
            Expects(p_ == p);
            p_ = nullptr;
        }

        [[nodiscard]] constexpr auto contains(const void* const in_ptr) noexcept
        {
            return in_ptr == p_;
        }

        [[nodiscard]] constexpr const auto& buffer() const noexcept { return buffer_; }

        [[nodiscard]] constexpr bool operator==(const single_stack_buffer& other) const noexcept
        {
            return this == &other;
        }

    private:
        void* p_ = nullptr;
        alignas(std::max_align_t) std::array<byte, size> buffer_{};
    };
}

#include "../compilation_config_out.h"