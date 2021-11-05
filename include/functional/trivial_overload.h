//
// Created by BlurringShadow on 2021-10-15.
//
#pragma once
#include "functional/invocable_obj.h"

namespace stdsharp::functional
{
    template<typename... Fns>
    struct trivial_overload : invocable_obj<Fns>...
    {
        template<typename... Args>
        constexpr explicit trivial_overload(Args&&... args) //
            noexcept((nothrow_constructible_from<invocable_obj<Fns>, Args> && ...)):
            invocable_obj<Fns>(::std::forward<Args>(args))...
        {
        }
    };

    template<typename... Fns>
    trivial_overload(Fns&&...) -> trivial_overload<::std::decay_t<Fns>...>;
}
