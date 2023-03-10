#pragma once

#include "../concepts/concepts.h"

namespace stdsharp
{
    template<typename T>
    struct value_wrapper
    {
        using value_type = T;

        value_wrapper() = default;

        template<typename... U>
            requires ::std::constructible_from<T, U...>
        constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
            value_(::std::forward<U>(u)...)
        {
        }

#define STDSHARP_OPERATOR(const_, ref)                             \
    constexpr const_ T ref value() const_ ref noexcept             \
    {                                                              \
        return static_cast<const_ T ref>(value_);                  \
    }                                                              \
    constexpr explicit operator const_ T ref() const_ ref noexcept \
    {                                                              \
        return value();                                            \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR

    private:
        T value_;
    };

    //     namespace details
    //     {
    //         template<typename T>
    //         struct inherit_wrapper : protected T
    //         {
    //             inherit_wrapper() = default;

    //             template<typename... U>
    //                 requires ::std::constructible_from<T, U...>
    //             constexpr inherit_wrapper(U&&... u): T(::std::forward<U>(u)...)
    //             {
    //             }
    //         };
    //     }

    //     template<inheritable T>
    //     struct value_wrapper<T> : private details::inherit_wrapper<T>
    //     {
    //         using details::inherit_wrapper<T>::inherit_wrapper;


    // #define STDSHARP_OPERATOR(const_, ref)                             \
    //     constexpr const_ T ref value() const_ ref noexcept             \
    //     {                                                              \
    //         return static_cast<const_ T ref>(*this);                   \
    //     }                                                              \
    //     constexpr explicit operator const_ T ref() const_ ref noexcept \
    //     {                                                              \
    //         return value();                                            \
    //     }

    //         STDSHARP_OPERATOR(, &)
    //         STDSHARP_OPERATOR(const, &)
    //         STDSHARP_OPERATOR(, &&)
    //         STDSHARP_OPERATOR(const, &&)

    // #undef STDSHARP_OPERATOR
    //     };

    template<typename T>
    value_wrapper(T&&) -> value_wrapper<T&&>;
}