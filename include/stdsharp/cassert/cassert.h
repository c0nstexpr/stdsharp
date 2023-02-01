#pragma once

#include <__concepts/constructible.h>
#include <cassert>

#include <concepts>

namespace stdsharp
{
    inline constexpr auto is_debug =
#ifdef NDEBUG
        false
#else
        true
#endif
        ;

    template<typename>
    constexpr void debug_throw(const auto&...)
    {
    }

    template<typename Exception, ::std::predicate Predicate, typename... Args>
        requires is_debug && ::std::constructible_from<Exception, Args...>
    [[noreturn]] constexpr void debug_throw(Predicate&& predicate, Args&&... args)
    {
        if(::std::invoke(static_cast<Predicate&&>(predicate)))
            throw Exception{static_cast<Args&&>(args)...};
    }
}