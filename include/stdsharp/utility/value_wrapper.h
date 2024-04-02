#pragma once

#include "../concepts/object.h"
#include "forward_cast.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename T>
    struct value_wrapper
    {
        T v{};
    };

    template<empty_type T>
    struct value_wrapper<T>
    {
        STDSHARP_NO_UNIQUE_ADDRESS T v{};
    };
}

namespace stdsharp
{
    template<typename T>
    struct value_wrapper : details::value_wrapper<T>
    {
        using value_type = T;

        value_wrapper() = default;

        template<typename... U>
            requires std::constructible_from<T, U...>
        constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
            details::value_wrapper<T>{cpp_forward(u)...}
        {
        }

        template<typename Self>
        [[nodiscard]] constexpr forward_cast_t<Self, T> get(this Self&& self) noexcept
        {
            return forward_cast<Self, value_wrapper>(self).v;
        }

        template<typename Self, typename SelfT = const Self>
        [[nodiscard]] constexpr forward_cast_t<SelfT, T> cget(this const Self&& self) noexcept
        {
            return forward_cast<SelfT, value_wrapper>(self).v;
        }

        template<typename Self, typename SelfT = const Self&>
        [[nodiscard]] constexpr forward_cast_t<SelfT, T> cget(this const Self& self) noexcept
        {
            return forward_cast<SelfT, value_wrapper>(self).v;
        }

        template<std::derived_from<value_wrapper> Self>
        [[nodiscard]] constexpr explicit operator forward_cast_t<Self, T>(this Self&& self) noexcept
        {
            return forward_cast<value_wrapper, Self>(self).get();
        }
    };

    template<typename T>
    value_wrapper(T&&) -> value_wrapper<std::decay_t<T>>;
}

#include "../compilation_config_out.h"