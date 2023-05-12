#pragma once

#include <algorithm>
#include <ranges>

#include "../utility/auto_cast.h"
#include "allocator_traits.h"
#include "../cmath/cmath.h"
#include "../compilation_config_in.h"

namespace stdsharp
{
    union all_aligned
    {
    private:
        ::std::max_align_t v;
    };

    template<::std::size_t Size>
    class static_memory_resource
    {
    public:
        static constexpr auto size = Size;

        using value_t = all_aligned;

        using states_t = ::std::array<bool, size>;

        using storage_t = ::std::array<all_aligned, size>;

        static_memory_resource() = default;

        static_memory_resource(const static_memory_resource&) = delete;

        constexpr static_memory_resource(static_memory_resource&&) noexcept { release(); }

        static_memory_resource& operator=(const static_memory_resource&) = delete;

        constexpr static_memory_resource& operator=(static_memory_resource&&) noexcept
        {
            release();
            return *this;
        }

        ~static_memory_resource() = default;

        [[nodiscard]] constexpr all_aligned* allocate(const ::std::size_t required_size)
        {
            const auto ptr = try_allocate(required_size);

            return (ptr == nullptr && required_size != 0) ? throw ::std::bad_alloc{} : ptr;
        }

        [[nodiscard]] constexpr all_aligned* try_allocate(const ::std::size_t required_size)
        {
            if(required_size == 0) return nullptr;

            const auto found = ::std::ranges::search_n(state_, auto_cast(required_size), false);

            if(found.empty()) return nullptr;

            ::std::ranges::fill(found, true);
            return storage_.data() + (found.begin() - state_.cbegin());
        }

        constexpr void
            deallocate(all_aligned* const ptr, const ::std::size_t required_size) noexcept
        {
            if(ptr == nullptr) return;
            ::std::ranges::fill_n(map_state(ptr), auto_cast(required_size), false);
        }

        constexpr void release() noexcept { state_ = {}; }

        [[nodiscard]] constexpr auto& state() const noexcept { return state_; }

        [[nodiscard]] constexpr auto& storage() const noexcept { return storage_; }

        [[nodiscard]] constexpr auto used() const noexcept
        {
            return ::std::ranges::count(state_, true);
        }

        [[nodiscard]] constexpr auto remaining() const noexcept { return size - used(); }

        [[nodiscard]] constexpr auto contains(const all_aligned* const ptr) noexcept
        {
            return map_state(ptr) != state_.cend();
        }

        [[nodiscard]] constexpr bool operator==(const static_memory_resource& other) const noexcept
        {
            return this == &other;
        }

    private:
        [[nodiscard]] constexpr auto map_state(const all_aligned* const ptr) noexcept
        {
            const auto diff = [&, this]
            {
                return ::std::is_constant_evaluated() ? //
                    constexpr_map_state_impl(ptr) :
                    ptr - storage_.data();
            }();

            return state_.begin() + ::std::ranges::min(diff, static_cast<decltype(diff)>(size));
        }

        [[nodiscard]] constexpr auto constexpr_map_state_impl(const all_aligned* const ptr) noexcept
        {
            const auto data = storage_.data();
            const auto ptr_view = ::std::views::iota(data, data + size);
            return ::std::ranges::find(ptr_view, ptr) - ptr_view.begin();
        }

        storage_t storage_{};

        states_t state_{};
    };

    template<typename T, ::std::size_t Size>
    using static_memory_resource_for =
        static_memory_resource<ceil_reminder(Size * sizeof(T), sizeof(all_aligned))>;

    template<::std::size_t Size>
    constexpr auto& get_static_memory_resource() noexcept
    {
        static thread_local constinit static_memory_resource<Size> resource{};
        return resource;
    }
}

#include "../compilation_config_out.h"