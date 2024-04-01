#pragma once

#include <functional>

#include "../macros.h"
#include "../namespace_alias.h"

namespace stdsharp
{
    inline constexpr struct invoke_fn
    {
        constexpr decltype(auto) operator()(auto&&... args) const noexcept(noexcept(std::invoke(cpp_forward(args)...)))
        {
            return std::invoke(cpp_forward(args)...);
        }
    } invoke{};
}