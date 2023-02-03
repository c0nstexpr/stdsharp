#pragma once

#include <algorithm>
#include <array>
#include <ranges>

#include "../utility/auto_cast.h"
#include "../cmath/cmath.h"

namespace stdsharp
{
    template<typename T, ::std::size_t Size, ::std::size_t Align = alignof(T)>
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

            const auto hint_begin = map_state(hint);

            const ::std::iter_difference_t<typename state_t::iterator> count_v = auto_cast(count);

            auto it = ::std::ranges::search_n(hint_begin, state_.cend(), count_v, false).begin();

            if(it == state_.cend())
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

        [[nodiscard]] constexpr bool operator==(const auto&) const noexcept { return false; }

        [[nodiscard]] constexpr const auto& state() const noexcept { return state_; }

        [[nodiscard]] constexpr const auto& storage() const noexcept { return storage_; }

        [[nodiscard]] constexpr auto used() const noexcept
        {
            return ::std::ranges::count(state_, true);
        }

        [[nodiscard]] constexpr auto max_size() const noexcept { return storage_.size() - used(); }

    private:
        [[nodiscard]] constexpr auto map_storage(const typename state_t::const_iterator it) noexcept
        {
            return storage_.begin() + (it - state_.cbegin());
        }

        [[nodiscard]] constexpr auto map_state(const void* ptr)
        {
            return ::std::is_constant_evaluated() ? constexpr_map_state_impl(ptr) :
                                                    map_state_impl(ptr);
        }

        [[nodiscard]] auto map_state_impl(const void* void_ptr)
        {
            const T* first = storage_.data();
            const T* ptr = auto_cast(void_ptr);

            return state_.begin() + // NOLINTNEXTLINE(*-pointer-arithmetic)
                (ptr >= first && ptr <= first + storage_.size() ? ptr - first : 0);
        }

        [[nodiscard]] constexpr auto constexpr_map_state_impl(const void* ptr)
        {
            const auto data = storage_.data(); // NOLINTNEXTLINE(*-pointer-arithmetic)
            const auto ptr_view = ::std::views::iota(data, data + storage_.size());
            const auto it = ::std::ranges::find(ptr_view, ptr);

            return state_.begin() + (it != ptr_view.end() ? it - ptr_view.begin() : 0);
        }

        union
        {
            alignas(Align) storage_t storage_{};
        };

        state_t state_{};
    };

    template<typename T>
    using sbo_allocator = static_allocator<
        T,
        ceil_reminder(alignof(::std::max_align_t), sizeof(T)),
        alignof(::std::max_align_t) // clang-format off
    >; // clang-format on
}