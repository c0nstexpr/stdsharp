#pragma once

#include <utility>

#include "../concepts/concepts.h"
#include "../compilation_config_in.h"
#include "stdsharp/type_traits/core_traits.h"

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
        [[nodiscard]] constexpr decltype(auto) get(this Self&& self) noexcept
        {
            return static_cast<cv_ref_align_t<Self&&,value_wrapper>>(cpp_forward(self)).v;
        }

        template<typename Self>
        [[nodiscard]] constexpr explicit operator cv_ref_align_t<Self&&, T>(this Self&& self) noexcept
        {
            return static_cast<cv_ref_align_t<Self&&,value_wrapper>>(cpp_forward(self)).get();
        }
    };

    template<typename T>
    value_wrapper(T&&) -> value_wrapper<std::decay_t<T>>;
}

#include "../compilation_config_out.h"