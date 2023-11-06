#pragma once

#include <cassert>
#include <gsl/assert>

#include "../namespace_alias.h"
#include "../concepts/concepts.h"

namespace stdsharp
{
    inline constexpr auto is_debug =
#ifdef NDEBUG
        false
#else
        true
#endif
        ;

    constexpr void assert_not_null(const nullable_pointer auto ptr) noexcept { Expects(ptr != nullptr); }
}
