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
        requires ::std::constructible_from<Exception, Args...> && ::std::predicate<const Predicate&>
    constexpr void precondition(
        const Predicate& predicate,
        [[maybe_unused]] Args&&... args
    ) noexcept(!is_debug && ::std::is_nothrow_invocable_r_v<bool, Predicate>)
    {
        if constexpr(is_debug)
            ::std::invoke(predicate) ? void() : throw Exception{static_cast<Args&&>(args)...};
        else STDSHARP_ASSUME(::std::invoke(predicate));
    }
}

#include "../compilation_config_out.h"