#pragma once

#include "../concepts/object.h"
#include "forward_like.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename T>
    class value_wrapper
    {
        T v_;

    public:
        template<typename... U>
            requires std::constructible_from<T, U...>
        constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
            v_(cpp_forward(u)...)
        {
        }

        template<typename Self>
        [[nodiscard]] constexpr decltype(auto) get(this Self&& self) noexcept
        {
            return forward_like<Self>(cpp_forward(self)).v_;
        }
    };

    template<typename T>
    class value_wrapper<T&> : std::reference_wrapper<T>
    {
        using m_base = std::reference_wrapper<T>;

    public:
        using m_base::m_base;

        [[nodiscard]] constexpr decltype(auto) get() const noexcept { return m_base::get(); }
    };

    template<empty_type T>
    class value_wrapper<T> : T
    {
    public:
        value_wrapper() = default;

        using T::T;

        template<typename... U>
            requires(sizeof...(U) > 0) && std::constructible_from<T, U...>
        constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
            T(cpp_forward(u)...)
        {
        }

        [[nodiscard]] constexpr decltype(auto) get(this auto&& self) noexcept
        {
            return forward_like<value_wrapper>(cpp_forward(self));
        }
    };

    template<void_ T>
    struct value_wrapper<T>
    {
        constexpr void get() const noexcept {}

        constexpr void cget() const noexcept {}
    };
}

namespace stdsharp
{
    template<typename T = void>
    struct value_wrapper : details::value_wrapper<T>
    {
    private:
        using m_base = details::value_wrapper<T>;

    public:
        using value_type = T;
        using m_base::m_base;
        using m_base::get;

        [[nodiscard]] constexpr decltype(auto) cget() const& noexcept
        {
            return get();
        }

        [[nodiscard]] constexpr decltype(auto) cget() const&& noexcept
        {
            return get();
        }
    };

    template<typename T>
    value_wrapper(T&&) -> value_wrapper<std::decay_t<T>>;

    value_wrapper() -> value_wrapper<>;
}

#include "../compilation_config_out.h"
