#pragma once

#include <algorithm>
#include <array>

#include <range/v3/view/concat.hpp>

#include "../utility/utility.h"

namespace stdsharp
{
    template<typename T, ::std::size_t Size>
    class static_allocator
    {
    public:
        using value_type = T;

        using state_t = ::std::array<bool, Size>;
        using storage_t = ::std::array<T, Size>;

        using propagate_on_container_copy_assignment = ::std::true_type;
        using propagate_on_container_move_assignment = ::std::true_type;
        using propagate_on_container_swap = ::std::true_type;

        static constexpr auto size = Size;

        [[nodiscard]] constexpr T* allocate(const ::std::size_t count)
        {
            return allocate(count, storage_.data());
        }

        [[nodiscard]] constexpr T* allocate(const ::std::size_t count, const void* hint)
        {
            using subrange = ::std::ranges::subrange<typename storage_t::const_iterator>;

            if(count == 0) return nullptr;

            const auto diff = static_cast<const T*>(hint) - storage_.data();

            const auto state_init_begin = state_.begin() + diff;

            const auto concat_view = ::ranges::views::concat(
                ::ranges::subrange{state_init_begin, state_.end()},
                ::ranges::subrange{state_.begin(), state_init_begin}
            );

            const ::std::ranges::range_difference_t<decltype(concat_view)> count_v =
                auto_cast(count);

            const auto it = ::std::ranges::search_n(concat_view, count_v, false).begin();

            if(it == concat_view.end()) throw ::std::bad_alloc{};

            ::std::ranges::fill_n(it, count_v, true);

            return &storage_[::std::ranges::distance(concat_view.begin(), it) + diff];
        }

        constexpr void deallocate(T* ptr, const ::std::size_t count) noexcept
        {
            if(ptr == nullptr) return;
            ::std::ranges::fill_n(state_.begin() + (ptr - storage_.data()), count, false);
        }

        constexpr bool operator==(const auto&) const noexcept { return false; }

        constexpr const auto& state() const noexcept { return state_; }

        constexpr const auto& storage() const noexcept { return storage_; }

        constexpr auto used() const noexcept { return ::std::ranges::count(state_, true); }

        constexpr auto max_size() const noexcept { return size - used(); }

    private:
        storage_t storage_{};

        state_t state_{};
    };
}