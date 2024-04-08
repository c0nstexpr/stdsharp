#pragma once

#include "../macros.h"
#include "../namespace_alias.h"

#include <functional>

namespace stdsharp
{
    inline constexpr struct invoke_fn
    {
        constexpr auto operator()(auto&&... args) const
            noexcept(noexcept(std::invoke(cpp_forward(args)...))) //
            -> decltype(std::invoke(cpp_forward(args)...))
        {
            return std::invoke(cpp_forward(args)...);
        }
    } invoke{};
}