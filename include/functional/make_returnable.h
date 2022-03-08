#pragma once

#include "functional/invoke.h"

namespace stdsharp::functional
{
    inline constexpr auto make_returnable = make_trivial_invocables(
        nodiscard_tag,
        []<typename Func> // clang-format off
            requires requires
            {
                ::std::bind(returnable_invoke, ::std::declval<Func>());
            }
        (Func&& func)
            noexcept(noexcept(::std::bind(returnable_invoke, ::std::forward<Func>(func))))
        {
            return ::std::bind(returnable_invoke, ::std::forward<Func>(func));
        } // clang-format on
    );
}