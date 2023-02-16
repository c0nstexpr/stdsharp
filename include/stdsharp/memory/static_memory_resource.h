#pragma once

#include <algorithm>
#include <array>
#include <ranges>

#include "../utility/auto_cast.h"
#include "allocator_traits.h"
#include "../cmath/cmath.h"

namespace stdsharp
{
    template<::std::size_t Size>
    class static_memory_resource
    {
    public:
        using value_type = ::std::byte;
        using state_t = ::std::array<bool, Size>;
        using storage_t = ::std::array<value_type, Size>;

        static_memory_resource() = default;

        constexpr static_memory_resource(const static_memory_resource&) noexcept {}

        static_memory_resource(static_memory_resource&&) noexcept = default;

        constexpr static_memory_resource& operator=(const static_memory_resource&) noexcept
        {
            state_.fill(false);
            return *this;
        }

        constexpr static_memory_resource& operator=(static_memory_resource&&) noexcept
        {
            state_.fill(false);
            return *this;
        }

        constexpr void swap(static_memory_resource&) noexcept { state_.fill(false); }

        ~static_memory_resource() = default;

        static constexpr auto size = Size;

        [[nodiscard]] constexpr value_type* allocate(
            const ::std::size_t required_size,
            const ::std::size_t alignment = max_alignment_v
        )
        {
            const auto ptr = try_allocate(required_size, alignment);
            return ptr == nullptr ? throw ::std::bad_alloc{} : ptr;
        }

        [[nodiscard]] constexpr value_type* try_allocate(
            const ::std::size_t required_size, // NOLINT(*-easily-swappable-parameters)
            const ::std::size_t alignment = max_alignment_v
        )
        {
            if(required_size == 0) return nullptr;

            for(::std::size_t i = 0; i != state_.size();)
            {
                auto next_i = i + alignment * ceil_reminder(required_size, alignment);
                const auto state_begin = state_.begin() + i;
                const auto state_end = state_.begin() + next_i;

                if(::std::ranges::search_n(state_begin, state_.begin() + next_i, true))
                {
                    ::std::ranges::fill(state_begin, state_end, true);
                    return storage_.data() + i;
                }

                i = next_i;
            }

            return nullptr;
        }

        constexpr void deallocate(value_type* ptr, const ::std::size_t required_size) noexcept
        {
            if(ptr == nullptr) return;
            ::std::ranges::fill_n(map_state(ptr), auto_cast(required_size), false);
        }

        [[nodiscard]] constexpr bool operator==(const auto&) const noexcept { return false; }

        [[nodiscard]] constexpr const auto& state() const noexcept { return state_; }

        [[nodiscard]] constexpr const auto& storage() const noexcept { return storage_; }

        [[nodiscard]] constexpr auto used() const noexcept { return state_.count(); }

        [[nodiscard]] constexpr auto max_size() const noexcept { return storage_.size() - used(); }

        [[nodiscard]] constexpr auto contains(const value_type* const ptr) noexcept
        {
            return map_state(ptr) != state_.cend();
        }

        constexpr bool operator==(const static_memory_resource& other) const noexcept
        {
            return this == &other;
        }

    private:
        [[nodiscard]] constexpr auto map_state(const void* ptr) noexcept
        {
            return state_.begin() +
                (::std::is_constant_evaluated() ? constexpr_map_state_impl(ptr) :
                                                  map_state_impl(ptr));
        }

        [[nodiscard]] auto map_state_impl(const void* void_ptr) noexcept
        {
            return ::std::ranges::min(
                static_cast<const value_type*>(void_ptr) - storage_.data(),
                static_cast<::std::ptrdiff_t>(storage_.size())
            );
        }

        [[nodiscard]] constexpr auto constexpr_map_state_impl(const void* ptr) noexcept
        {
            const auto data = storage_.data();
            const auto ptr_view = ::std::views::iota(data, data + storage_.size());
            const auto diff = ::std::ranges::find(ptr_view, ptr) - ptr_view.begin();

            return ::std::ranges::min(diff, static_cast<decltype(diff)>(storage_.size()));
        }

        union
        {
            alignas(::std::max_align_t) storage_t storage_{};
        };

        state_t state_{};
    };
}