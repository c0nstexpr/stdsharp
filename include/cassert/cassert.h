#pragma once

#include <cassert>

#include "type_traits/core_traits.h"

namespace stdsharp
{
    inline constexpr auto is_debug =
#ifdef NDEBUG
        false
#else
        true
#endif
        ;

    inline constexpr auto unreachable = [](const auto&...) constexpr noexcept
    {
        struct // NOLINT
        {
            int _;
        } _;
        ++_._;
    };
}
