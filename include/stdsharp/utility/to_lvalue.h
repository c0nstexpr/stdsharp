#pragma once

#include "../macros.h"
#include "../compilation_config_in.h"
#include "../concepts/concepts.h"

namespace stdsharp
{
    inline constexpr struct to_lvalue_fn
    {
        STDSHARP_INTRINSIC constexpr auto& operator()(auto& t) const noexcept { return t; }

        template<std::move_constructible T>
        [[nodiscard]] constexpr T operator()(T&& t) const noexcept(nothrow_move_constructible<T>)
        {
            return cpp_move(t);
        }
    } to_lvalue{};

    template<typename T>
    using to_lvalue_t = std::invoke_result_t<to_lvalue_fn, T>;
}

#include "../compilation_config_out.h"