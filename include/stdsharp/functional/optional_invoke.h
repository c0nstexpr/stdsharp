#pragma once

#include "empty_invoke.h"
#include "invoke.h"
#include "sequenced_invocables.h"

namespace stdsharp
{
    inline constexpr sequenced_invocables optional_invoke{invoke, empty_invoke};

    template<typename... Args>
    concept nothrow_optional_invocable = noexcept(optional_invoke(std::declval<Args>()...));
}