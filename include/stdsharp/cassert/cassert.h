#pragma once

#include <cassert>

#include "../details/prologue.h"

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

#include "../details/epilogue.h"