#pragma once

#include "allocator_traits.h"
#include "../cassert/cassert.h"
#include "../cstdint/cstdint.h"

namespace stdsharp
{
    template<std::size_t Size>
    class static_memory_resource
    {
    public:
        static constexpr auto size = Size;

        using states_t = std::array<bool, size>;
        using storage_t = std::array<byte, size>;

        static_memory_resource() = default;

        static_memory_resource(const static_memory_resource&) = delete;

        constexpr static_memory_resource(static_memory_resource&& /*unused*/) noexcept
        {
            release();
        }

        static_memory_resource& operator=(const static_memory_resource&) = delete;

        constexpr static_memory_resource& operator=(static_memory_resource&& /*unused*/) noexcept
        {
            release();
            return *this;
        }

        ~static_memory_resource() = default;

        [[nodiscard]] constexpr auto
            try_aligned_allocate(const std::align_val_t align, const std::size_t required_size)
        {
            if(required_size == 0) return nullptr;

            const auto size =
                static_cast<std::ranges::range_difference_t<decltype(state_)>>(required_size);

            if(std::is_constant_evaluated() || align == std::align_val_t{1})
            {
                const auto found = std::ranges::search_n(state_, size, false);

                if(found.empty()) return nullptr;

                std::ranges::fill(found, true);

                return storage_.data(), +std::ranges::distance(state_.cbegin(), found.begin());
            }

            for(auto begin = state_.begin(); begin != state_.cend();)
            {
                auto&& found = std::ranges::search_n(state_, size, false);

                if(found.empty()) return nullptr;

                auto&& [found_begin, found_end] = cpp_move(found);
                auto&& remained_end = std::ranges::find(cpp_move(found_end), state_.cend(), true);
                const auto ptr = std::align(
                    auto_cast(align),
                    remained_end - found_begin,
                    std::to_address(begin),
                    size
                );

                if(ptr == nullptr) begin = cpp_move(remained_end);
                else
                {
                    std::ranges::fill(std::to_address(found_begin), ptr, true);
                    return ptr;
                }
            }

            return nullptr;
        }

        [[nodiscard]] constexpr auto try_allocate(const std::size_t required_size)
        {
            return try_aligned_allocate(1, required_size);
        }

        [[nodiscard]] constexpr auto allocate(const std::size_t required_size)
        {
            const auto ptr = try_allocate(required_size);

            return (ptr == nullptr && required_size != 0) ? throw std::bad_alloc{} : ptr;
        }

        [[nodiscard]] constexpr auto aligned_allocate(
            const std::align_val_t align,
            const std::size_t required_size
        )
        {
            const auto ptr = try_aligned_allocate(align, required_size);

            return (ptr == nullptr && required_size != 0) ? throw std::bad_alloc{} : ptr;
        }

        constexpr void deallocate(byte* const ptr, const std::size_t required_size) noexcept
        {
            if(ptr == nullptr) return;
            Expects(contains(ptr));
            std::ranges::fill_n(map_state(ptr), auto_cast(required_size), false);
        }

        constexpr void release() noexcept { state_ = {}; }

        [[nodiscard]] constexpr auto& state() const noexcept { return state_; }

        [[nodiscard]] constexpr auto& storage() const noexcept { return storage_; }

        [[nodiscard]] constexpr auto used() const noexcept
        {
            return std::ranges::count(state_, true);
        }

        [[nodiscard]] constexpr auto remaining() const noexcept { return size - used(); }

        [[nodiscard]] constexpr auto contains(const void* const ptr) noexcept
        {
            return std::is_constant_evaluated() ? //
                constexpr_map_state_impl(ptr) < size :
                (ptr >= std::to_address(storage_.cbegin())) &&
                    (ptr < std::to_address(storage_.cend()));
        }

        [[nodiscard]] constexpr bool operator==(const static_memory_resource& other) const noexcept
        {
            return this == &other;
        }

    private:
        [[nodiscard]] constexpr auto map_state(const byte* const ptr) noexcept
        {
            const auto diff = [&, this]
            {
                return std::is_constant_evaluated() ? //
                    constexpr_map_state_impl(ptr) :
                    ptr - storage_.data();
            }();

            return state_.begin() + std::ranges::min(diff, static_cast<decltype(diff)>(size));
        }

        [[nodiscard]] constexpr auto constexpr_map_state_impl(const void* const ptr) noexcept
        {
            const auto data = storage_.data();
            const auto ptr_view = std::views::iota(data, std::ranges::next(data, size));
            return std::ranges::find(ptr_view, ptr) - ptr_view.begin();
        }

        storage_t storage_{};

        states_t state_{};
    };

    template<typename T, std::size_t Size>
    using static_memory_resource_for = static_memory_resource<Size * sizeof(T)>;
}
