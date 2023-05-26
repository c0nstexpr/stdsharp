#pragma once

#include "../concepts/concepts.h"

namespace stdsharp
{
    template<typename T = void>
    struct constructor
    {
        template<typename... Args>
            requires std::constructible_from<T, Args...>
        [[nodiscard]] constexpr auto operator()(Args&&... args) const
            noexcept(nothrow_constructible_from<T, Args...>)
        {
            return T(cpp_forward(args)...);
        }

        template<typename... Args>
            requires(!std::constructible_from<T, Args...> && list_initializable_from<T, Args...>)
        [[nodiscard]] constexpr auto operator()(Args&&... args) const
            noexcept(nothrow_list_initializable_from<T, Args...>)
        {
            return T{cpp_forward(args)...};
        }

        template<typename U>
            requires std::constructible_from<std::decay_t<U>, U> && std::same_as<T, void>
        [[nodiscard]] constexpr std::decay_t<U> operator()(U&& u) const
            noexcept(nothrow_constructible_from<std::decay_t<U>, U>)
        {
            return std::decay_t<U>{cpp_forward(u)};
        }
    };

    template<typename T = void>
    inline constexpr constructor<T> construct{};
}