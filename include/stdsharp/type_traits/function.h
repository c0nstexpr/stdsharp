
#pragma once

#include "../functional/invoke.h"

namespace stdsharp
{
    namespace details
    {
        template<typename R, bool Noexcept, typename... Args>
        struct function_traits_helper_base
        {
            static auto constexpr is_noexcept = Noexcept;

            using result_t = R;
            using args_t = regular_type_sequence<Args...>;
            using function_t = R(Args...) noexcept(Noexcept);
            using type = R (*)(Args...) noexcept(Noexcept);
        };
    }

    template<typename>
    struct function_traits;

    template<typename R, typename... Args>
    struct function_traits<R (*)(Args...)> : function_traits<R(Args...)>
    {
    };

    template<typename R, typename... Args>
    struct function_traits<R (*)(Args...) noexcept> : function_traits<R(Args...) noexcept>
    {
    };

    template<typename R, typename... Args>
    struct function_traits<R (&)(Args...)> : function_traits<R(Args...)>
    {
    };

    template<typename R, typename... Args>
    struct function_traits<R (&)(Args...) noexcept> : function_traits<R(Args...) noexcept>
    {
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) noexcept> :
        details::function_traits_helper_base<R, true, Args...>
    {
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...)> : details::function_traits_helper_base<R, true, Args...>
    {
    };

    template<auto Ptr>
    using function_pointer_traits = function_traits<decltype(Ptr)>;
}