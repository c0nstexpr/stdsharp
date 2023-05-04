#pragma once

#include "../concepts/concepts.h"

namespace stdsharp
{
    template<typename U>
    struct cast_to_fn
    {
        template<typename T>
            requires requires { static_cast<U>(::std::declval<T>()); }
        [[nodiscard]] constexpr U operator()(T&& t) const
            noexcept(noexcept(static_cast<U>(::std::declval<T>())))
        {
            return static_cast<U>(cpp_forward(t));
        }
    };

    template<typename U>
    inline constexpr cast_to_fn<U> cast_to{};
}