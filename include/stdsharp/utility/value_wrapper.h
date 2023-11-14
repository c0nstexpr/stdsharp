#pragma once

#include <utility>

#include "../concepts/concepts.h"
#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename T>
    struct value_wrapper
    {
        T v_;

        value_wrapper() = default;

        template<typename... U>
            requires std::constructible_from<T, U...>
        constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
            v_(cpp_forward(u)...)
        {
        }
    };

    template<empty_type T>
    struct value_wrapper<T>
    {
        STDSHARP_NO_UNIQUE_ADDRESS T v_;

        value_wrapper() = default;

        template<typename... U>
            requires std::constructible_from<T, U...>
        constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
            v_(cpp_forward(u)...)
        {
        }
    };
}

namespace stdsharp
{
    template<typename T>
    struct value_wrapper : details::value_wrapper<T>
    {
        using value_type = T;
        using details::value_wrapper<T>::value_wrapper;

#define STDSHARP_OPERATOR(cv, ref)                         \
    [[nodiscard]] constexpr cv T ref get() cv ref noexcept \
    {                                                      \
        return static_cast<cv T ref>(this->v_);            \
    }                                                      \
                                                           \
    [[nodiscard]] constexpr explicit operator cv T ref() cv ref noexcept { return get(); }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)
        STDSHARP_OPERATOR(volatile, &)
        STDSHARP_OPERATOR(const volatile, &)
        STDSHARP_OPERATOR(volatile, &&)
        STDSHARP_OPERATOR(const volatile, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename T>
    value_wrapper(T&&) -> value_wrapper<std::decay_t<T>>;
}

#include "../compilation_config_out.h"