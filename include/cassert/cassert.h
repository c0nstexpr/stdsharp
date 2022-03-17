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

    template<auto Message>
    inline constexpr auto unreachable = [] constexpr noexcept
    {
        struct
        {
            int _;
        } _;
        [[maybe_unused]] auto v = _._;
    };
}
