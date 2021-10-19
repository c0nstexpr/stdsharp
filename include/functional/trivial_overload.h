//
// Created by BlurringShadow on 2021-10-15.
//

#include "functional/invocable_obj.h"

namespace stdsharp::functional
{
    template<typename... Fns>
    struct trivial_overload : ::stdsharp::functional::invocable_obj<Fns>...
    {
        template<typename... Args>
        constexpr explicit trivial_overload(Args&&... args) //
            noexcept(
                (::stdsharp::concepts::
                     nothrow_constructible_from<::stdsharp::functional::invocable_obj<Fns>, Args> &&
                 ...) // clang-format off
            ):
            // clang-format on
            ::stdsharp::functional::invocable_obj<Fns>(::std::forward<Args>(args))...
        {
        }
    };

    template<typename... Fns>
    trivial_overload(Fns&&...) -> trivial_overload<::std::decay_t<Fns>...>;
}
