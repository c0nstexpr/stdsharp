#pragma once

#include <cassert>

namespace stdsharp::cassert
{
    inline constexpr auto is_debug =
#ifdef NDEBUG
        false
#else
        true
#endif
        ;
}
