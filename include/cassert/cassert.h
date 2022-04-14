#pragma once

#include <cassert>

namespace stdsharp
{
    inline constexpr auto is_debug =
#ifdef NDEBUG
        false
#else
        true
#endif
        ;

    inline constexpr auto unreachable = []() constexpr noexcept
    {
        int _; // NOLINT(*-init-variables)
        ++_;
    };
}
