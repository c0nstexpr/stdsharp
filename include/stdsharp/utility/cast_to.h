#pragma once

#include "../concepts/concepts.h"
#include "../type_traits/core_traits.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename U>
    struct cast_to_fn
    {
        STDSHARP_INTRINSIC constexpr U operator()(explicitly_convertible<U> auto&& t) const
            noexcept(noexcept(static_cast<U>(cpp_forward(t))))
        {
            return static_cast<U>(cpp_forward(t));
        }
    };

    template<typename U>
    inline constexpr cast_to_fn<U> cast_to{};

    template<typename T>
    struct cast_fwd_fn
    {
        template<typename U>
            requires explicitly_convertible<U, cv_ref_align_t<U&&, T>>
        STDSHARP_INTRINSIC constexpr decltype(auto) operator()(U&& u) const
            noexcept(nothrow_explicitly_convertible<U, cv_ref_align_t<U&&, T>>)
        {
            return static_cast<cv_ref_align_t<U&&, T>>(cpp_forward(u));
        }
    };

    template<typename T>
    inline constexpr auto cast_fwd = cast_fwd_fn<T>{};
}

#include "../compilation_config_out.h"