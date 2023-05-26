#pragma once

#include <cassert>
#include <functional>

#include "../compilation_config_in.h"

namespace stdsharp
{
    inline constexpr auto is_debug =
#ifdef NDEBUG
        false
#else
        true
#endif
        ;

    template<typename Exception, typename Predicate, typename... Args>
        requires std::constructible_from<Exception, Args...> && std::predicate<const Predicate&> &&
        is_debug
    constexpr void precondition(const Predicate& predicate, Args&&... args)
    {
        std::invoke(predicate) ? void() : throw Exception{static_cast<Args&&>(args)...}; // NOLINT
    }

    template<typename, typename Predicate>
        requires std::predicate<const Predicate&>
    constexpr void precondition([[maybe_unused]] const Predicate& predicate, auto&&...) noexcept
    {
        STDSHARP_ASSUME(std::invoke(predicate));
    }
}

#include "../compilation_config_out.h"