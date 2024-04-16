#pragma once

#include "../type_traits/object.h"

namespace stdsharp
{
    inline constexpr struct empty_invoke_fn
    {
        constexpr empty_t operator()(const auto&... /*unused*/) const noexcept { return {}; }
    } empty_invoke{};
}