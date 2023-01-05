#pragma once

#include <array>

#include "../utility/utility.h"
#include "../algorithm/algorithm.h"

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

        template<typename U>
        struct rebind
        {
            using other = static_allocator<U, Size>;
        };

        static constexpr auto size = Size;

        [[nodiscard]] constexpr T* allocate(const ::std::size_t count)
        {
            return allocate(count, storage_.data());
        }

        [[nodiscard]] constexpr T* allocate(const ::std::size_t count, const void* hint)
        {
            if(count == 0) return nullptr;

            const auto hint_begin = map_state(auto_cast(hint));

            const std::iter_difference_t<typename state_t::iterator> count_v = auto_cast(count);

            auto it = ::std::ranges::search_n(hint_begin, state_.cend(), count_v, false).begin();

            if(it == state_.end())
            {
                it = ::std::ranges::search_n( // clang-format off
                    state_.begin(),
                    hint_begin,
                    count_v,
                    false
                ).begin(); // clang-format on

                if(it == hint_begin) throw ::std::bad_alloc{};
            }

            ::std::ranges::fill_n(it, count_v, true);

            return &*map_storage(it);
        }

        constexpr void deallocate(T* ptr, const ::std::size_t count) noexcept
        {
            if(ptr == nullptr) return;
            ::std::ranges::fill_n(map_state(ptr), auto_cast(count), false);
        }

        constexpr bool operator==(const auto&) const noexcept { return false; }

        constexpr const auto& state() const noexcept { return state_; }

        constexpr const auto& storage() const noexcept { return storage_; }

        constexpr auto used() const noexcept { return ::std::ranges::count(state_, true); }

        constexpr auto max_size() const noexcept { return storage_.size() - used(); }

    private:
        constexpr auto map_storage(const typename state_t::const_iterator it) noexcept
        {
            return storage_.begin() + (it - state_.cbegin());
        }

        constexpr auto map_state(const T* ptr)
        {
            const auto diff = ptr - storage_.data();

            return is_between(diff, 0, storage_.size()) ? state_.begin() + diff : state_.end();
        }

        storage_t storage_{};

        state_t state_{};
    };
}