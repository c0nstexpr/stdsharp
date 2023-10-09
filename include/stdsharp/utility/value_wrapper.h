#pragma once

#include <utility>

#include "../concepts/concepts.h"
#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename T>
    struct tuple_wise_ctor
    {
    private:
        template<size_t... I>
        constexpr tuple_wise_ctor(auto&& args, const std::index_sequence<I...>):
            T(std::get<I>(cpp_forward(args))...)
        {
        }

    public:
        tuple_wise_ctor() = default;

        template<typename... Args, typename Tuple = std::tuple<Args...>>
            requires std::is_constructible_v<T, Args...> &&
            (!std::is_constructible_v<T, const std::piecewise_construct_t&, Tuple>)
        constexpr tuple_wise_ctor(const std::piecewise_construct_t, Tuple&& args) //
            noexcept(nothrow_constructible_from<T, Args...>):
            tuple_wise_ctor(cpp_forward(args), std::index_sequence_for<Args...>{})
        {
        }
    };

    namespace details
    {
        template<typename T>
        struct value_wrapper
        {
            T v;

            value_wrapper() = default;

            template<typename... U>
                requires std::constructible_from<T, U...>
            constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
                v(cpp_forward(u)...)
            {
            }
        };

        template<empty_type T>
        struct value_wrapper<T>
        {
            STDSHARP_NO_UNIQUE_ADDRESS T v;

            value_wrapper() = default;

            template<typename... U>
                requires std::constructible_from<T, U...>
            constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
                v(cpp_forward(u)...)
            {
            }
        };
    }

    template<typename T>
    struct STDSHARP_EBO value_wrapper : details::value_wrapper<T>, tuple_wise_ctor<value_wrapper<T>>
    {
        using value_type = T;
        using details::value_wrapper<T>::value_wrapper;
        using tuple_wise_ctor<value_wrapper<T>>::tuple_wise_ctor;
        using details::value_wrapper<T>::v;

#define STDSHARP_OPERATOR(cv, ref)                                                              \
    [[nodiscard]] constexpr cv T ref get() cv ref noexcept { return static_cast<cv T ref>(v); } \
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