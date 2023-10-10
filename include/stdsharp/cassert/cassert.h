#pragma once

#include <cassert>
#include <concepts>
#include <gsl/assert>

#include "../namespace_alias.h"

namespace stdsharp
{
    inline constexpr auto is_debug =
#ifdef NDEBUG
        false
#else
        true
#endif
        ;
}
