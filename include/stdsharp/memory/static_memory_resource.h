#pragma once

#include <algorithm>
#include <ranges>
#include <array>

#include <range/v3/view/concat.hpp>

#include "../cstdint/cstdint.h"

namespace stdsharp
{
    template<::std::size_t Size>
    class static_memory_resource
    {
    public:
        using state_t = ::std::array<bool, Size>;
        using storage_t = ::std::array<byte, Size>;

        static constexpr auto size = Size;

        [[nodiscard]] constexpr void* allocate(const ::std::size_t bytes)
        {
            return allocate(bytes, storage_.begin());
        }

        [[nodiscard]] constexpr void*
            allocate(const ::std::size_t bytes, const typename storage_t::const_iterator hint)
        {
            using subrange = ::std::ranges::subrange<typename storage_t::const_iterator>;

            if(bytes == 0) return nullptr;

            const auto state_init_begin = state_.begin() + (hint - storage_.cbegin());

            const auto it = ::std::ranges::search_n(
                ::ranges::views::concat(
                    subrange{state_init_begin, state_.end()},
                    subrange{state_.begin(), state_init_begin}
                ),
                bytes,
                false
            );

            ::std::ranges::fill_n(it, bytes, true);

            return &storage_[it - state_.cbegin()];
        }

        constexpr void deallocate(void* ptr, const ::std::size_t bytes) noexcept
        {
            if(ptr == nullptr) return;

            const auto index = static_cast<byte*>(ptr) - storage_.data();

            if(index >= storage_.size())
                ::std::ranges::fill_n(state_.begin() + index, bytes, false);
        }

        constexpr bool operator==(const auto&) const noexcept { return false; }

        constexpr const auto& state() const noexcept { return state_; }

        constexpr const auto& storage() const noexcept { return storage_; }

    private:
        storage_t storage_{};

        state_t state_{};
    };
}