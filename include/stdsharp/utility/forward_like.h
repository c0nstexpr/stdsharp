#pragma once
#include <utility>

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename T>
    struct forward_like_fn
    {
        template<typename U>
        STDSHARP_INTRINSIC constexpr decltype(auto) operator()(U&& u) noexcept
        {
            return std::forward_like<T, U>(u);
        }
    };

    template<typename T>
    inline constexpr forward_like_fn<T> forward_like{};

    template<typename From, typename To>
    using forward_like_t = std::invoke_result_t<forward_like_fn<From>, To>;
}

#include "../compilation_config_out.h"
