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

    template<typename Exception, ::std::predicate Predicate, typename... Args>
        requires ::std::constructible_from<Exception, Args...>
    constexpr void precondition(
        Predicate&& predicate,
        Args&&... args
    ) noexcept(!is_debug && ::std::is_nothrow_invocable_r_v<bool, Predicate>)
    {
        if constexpr(is_debug)
            ::std::invoke(static_cast<Predicate&&>(predicate)) ?
                void() :
                throw Exception{static_cast<Args&&>(args)...};
        else STDSHARP_ASSUME(::std::invoke(static_cast<Predicate&&>(predicate)));
    }
}

#include "../compilation_config_out.h"