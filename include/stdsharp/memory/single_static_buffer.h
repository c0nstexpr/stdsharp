#pragma once

#include <memory>

#include "../cstdint/cstdint.h"
#include "../type_traits/object.h"

namespace stdsharp
{
    class bad_static_buffer_access : public ::std::exception
    {
    public:
        [[nodiscard]] constexpr const char* what() const noexcept override
        {
            return "bad static buffer access";
        }
    };

    template<::std::size_t Size>
    class single_static_buffer // NOLINTBEGIN(*-reinterpret-cast)
    {
    public:
        template<typename T>
        class [[nodiscard]] element : unique_object // NOLINT(*-special-member-functions)
        {
            friend class single_static_buffer;

            template<typename... Args>
            constexpr element(
                single_static_buffer& instance,
                Args&&... args
            ) noexcept(nothrow_invocable<decltype(::std::ranges::construct_at), T*, Args...>):
                instance_(&instance)
            {
                ::std::ranges::construct_at(ptr(), ::std::forward<Args>(args)...);

                instance.used_ = true;
            }

        public:
            element() = default;

            [[nodiscard]] constexpr T& ref() const noexcept { return *ptr_impl(); }

            [[nodiscard]] constexpr T* ptr() const noexcept
            {
                return instance_ == nullptr ? nullptr : ptr_impl();
            }

            [[nodiscard]] constexpr operator bool() const noexcept { return instance_ != nullptr; }

            constexpr ~element() noexcept
                requires ::std::invocable<decltype(::std::ranges::destroy_at), T*>
            {
                if(instance_ == nullptr) return;
                ::std::ranges::destroy_at(ptr());
                instance().used_ = false;
            }

        private:
            [[nodiscard]] constexpr T* ptr_impl() const noexcept
            {
                return reinterpret_cast<T*>(instance().storage_.data());
            }

            [[nodiscard]] constexpr auto& instance() const noexcept { return *instance_; }

            single_static_buffer* instance_ = nullptr;
        };

        using storage_t = ::std::array<byte, Size>;

        [[nodiscard]] static constexpr ::std::size_t size() noexcept { return Size; }

        [[nodiscard]] constexpr bool is_used() noexcept { return used_; }

        template<typename T, typename... Args>
            requires ::std::invocable<decltype(::std::ranges::construct_at), T*, Args...>
        [[nodiscard]] constexpr element<T> construct(Args&&... args)
        {
            if(sizeof(T) > size() || is_used()) throw ::std::bad_alloc{};

            return {*this, ::std::forward<Args>(args)...};
        }

    private:
        storage_t storage_{};

        bool used_{};
    }; // NOLINTEND(*-reinterpret-cast)
}