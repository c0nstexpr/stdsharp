#pragma once

#include "invoke.h"

namespace stdsharp::functional
{
    inline constexpr nodiscard_invocable make_returnable = []<typename Func>
        requires requires { ::std::bind(returnable_invoke, ::std::declval<Func>()); }
    (Func&& func) noexcept(noexcept(::std::bind(returnable_invoke, ::std::forward<Func>(func))))
    {
        return ::std::bind(returnable_invoke, ::std::forward<Func>(func));
    };
}