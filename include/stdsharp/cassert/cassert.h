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

    template<typename Exception, typename Predicate, typename... Args>
        requires std::constructible_from<Exception, Args...> && std::predicate<Predicate>
    constexpr void precondition(Predicate predicate, Args&&... args)
    {
        if constexpr(is_debug)
            if(!std::invoke(predicate)) throw Exception{static_cast<Args&&>(args)...};
    }
}
