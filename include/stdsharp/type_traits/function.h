
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
            using ptr_t = function_t*;
        };

        template<typename T>
        struct function_traits_helper;

        template<typename R, typename... Args>
        struct function_traits_helper<R (*)(Args...) noexcept(false)> :
            function_traits_helper_base<R, false, Args...>
        {
        };

        template<typename R, typename... Args>
        struct function_traits_helper<R (*)(Args...) noexcept(true)> :
            function_traits_helper_base<R, true, Args...>
        {
        };
    }

    template<typename T>
    struct function_traits : details::function_traits_helper<::std::decay_t<T>>
    {
        using type = ::std::decay_t<T>;
    };

    template<auto Ptr>
    using function_pointer_traits = function_traits<decltype(Ptr)>;
}