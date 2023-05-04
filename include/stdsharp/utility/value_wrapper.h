#pragma once

#include "../concepts/concepts.h"
#include "../compilation_config_in.h"

namespace stdsharp
{
    namespace details
    {
        template<typename T>
        struct value_wrapper
        {
            T v;
        };

        template<empty_type T>
        struct value_wrapper<T>
        {
            STDSHARP_NO_UNIQUE_ADDRESS T v;
        };
    }

    template<typename T>
    struct value_wrapper : details::value_wrapper<T>
    {
        using value_type = T;
        using details::value_wrapper<T>::v;

        value_wrapper() = default;

        template<typename... U>
            requires ::std::constructible_from<T, U...>
        constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
            details::value_wrapper<T>{cpp_forward(u)...}
        {
        }

#define STDSHARP_OPERATOR(const_, ref)                                                          \
    constexpr const_ T ref value() const_ ref noexcept { return static_cast<const_ T ref>(v); } \
                                                                                                \
    constexpr explicit operator const_ T ref() const_ ref noexcept { return value(); }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };
}

#include "../compilation_config_out.h"