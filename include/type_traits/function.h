// Created by BlurringShadow at 2021-03-03-下午 4:33

#pragma once
#include "type_sequence.h"

namespace stdsharp::type_traits
{
    namespace details
    {
        template<typename T>
        struct function_traits_helper;

        template<bool IsNoexcept>
        struct function_qualifiers_traits
        {
            static auto constexpr is_noexcept = IsNoexcept;
        };

        template<typename R, typename... Args>
        struct function_traits_helper_base
        {
            using result_t = R;
            using args_t = type_sequence<Args...>;
        };

        template<typename R, typename... Args>
        struct function_traits_helper<R (*)(Args...)> :
            details::function_traits_helper_base<R, Args...>,
            details::function_qualifiers_traits<false>
        {
        };

        template<typename R, typename... Args>
        struct function_traits_helper<R (*)(Args...) noexcept> :
            details::function_traits_helper_base<R, Args...>,
            function_qualifiers_traits<true>
        {
        };
    }

    template<typename T>
    struct function_traits : details::function_traits_helper<::std::decay_t<T>>
    {
    };

    template<auto Ptr>
    struct function_pointer_traits : function_traits<::std::decay_t<decltype(Ptr)>>
    {
    };
}
