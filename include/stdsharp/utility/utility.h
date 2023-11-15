
#pragma once

#include "auto_cast.h"
#include "cast_to.h"
#include "constructor.h"
#include "to_lvalue.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename T>
    struct forward_like_fn
    {
        [[nodiscard]] constexpr decltype(auto) operator()(auto&& u) const noexcept
        {
            return std::forward_like<T>(u);
        }
    };

    template<typename T>
    inline constexpr forward_like_fn<T> forward_like{};

    template<typename T, typename U>
    using forward_like_t = decltype(forward_like<T>(std::declval<U>()));
}

#include "../compilation_config_out.h"