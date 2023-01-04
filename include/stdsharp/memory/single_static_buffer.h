#pragma once

#include <memory>

#include "../cstdint/cstdint.h"
#include "../type_traits/object.h"
#include "../scope.h"

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
        using storage_t = ::std::array<byte, Size>;

        [[nodiscard]] constexpr bool is_used() noexcept { return used_; }

        template<typename T, typename... Args>
            requires ::std::invocable<decltype(::std::ranges::construct_at), T*, Args...>
        [[nodiscard]] constexpr auto construct(Args&&... args)
        {
            class raii_guard : unique_object // NOLINT(*-special-member-functions)
            {
            public:
                constexpr raii_guard(
                    single_static_buffer& instance,
                    const private_object<single_static_buffer> = {}
                ):
                    instance_(instance)
                {
                    if(instance.used_) throw ::std::bad_alloc{};

                    ::std::ranges::construct_at(ptr(), ::std::forward<Args>(args)...);

                    instance.used_ = true;
                }

                constexpr T& ref() const noexcept { return *ptr(); }

                constexpr T* ptr() const noexcept
                {
                    return reinterpret_cast<T*>(instance_.get().storage_.data());
                }

                constexpr ~raii_guard() noexcept
                    requires ::std::invocable<decltype(::std::ranges::destroy_at), T*>
                {
                    ::std::ranges::destroy_at(ptr());
                    instance_.get().used_ = false;
                }

            private:
                ::std::reference_wrapper<single_static_buffer> instance_;
            };

            return raii_guard{*this};
        }

    private:
        storage_t storage_{};

        bool used_{};
    }; // NOLINTEND(*-reinterpret-cast)
}