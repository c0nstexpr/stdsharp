#pragma once
#include <utility>

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename T, typename U>
    using forward_like_t = decltype(std::forward_like<T>(std::declval<U>()));

    template<typename T, typename U>
    struct forward_like_fn
    {
        STDSHARP_INTRINSIC constexpr decltype(auto) operator()(auto&& u) noexcept
        {
            return (forward_like_t<T, U>)u;
        }
    };

    template<typename T>
    struct forward_like_fn<T, void>
    {
        template<typename U>
        STDSHARP_INTRINSIC constexpr decltype(auto) operator()(U&& u) noexcept
        {
            return forward_like_fn<T, U>::operator()(u);
        }
    };

    template<typename T, typename U = void>
    inline constexpr forward_like_fn<T, U> forward_like{};
}

#include "../compilation_config_out.h"
