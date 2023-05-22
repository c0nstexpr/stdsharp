#pragma once

#include "../concepts/concepts.h"
#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename U>
    struct cast_to_fn
    {
        STDSHARP_INTRINSIC [[nodiscard]] constexpr U operator()( //
            explicitly_convertible<U> auto&& t
        ) const noexcept(noexcept(static_cast<U>(cpp_forward(t))))
        {
            return static_cast<U>(cpp_forward(t));
        }
    };

    template<typename U>
    inline constexpr cast_to_fn<U> cast_to{};

    template<typename T>
    struct cast_fwd_fn
    {
#define STDSHARP_OPERATOR(const_, volatile_, ref)                                            \
    STDSHARP_INTRINSIC constexpr const_ volatile_ T ref operator()(const_ volatile_ T ref t) \
        const noexcept                                                                       \
    {                                                                                        \
        return static_cast<const_ volatile_ T ref>(t);                                       \
    }

        STDSHARP_OPERATOR(const, , &)
        STDSHARP_OPERATOR(const, , &&)
        STDSHARP_OPERATOR(const, volatile, &)
        STDSHARP_OPERATOR(const, volatile, &&)
        STDSHARP_OPERATOR(, volatile, &)
        STDSHARP_OPERATOR(, volatile, &&)
        STDSHARP_OPERATOR(, , &)
        STDSHARP_OPERATOR(, , &&)

#undef STDSHARP_OPERATOR
    };

    template<typename T>
    inline constexpr auto cast_fwd = cast_fwd_fn<T>{};
}

#include "../compilation_config_out.h"