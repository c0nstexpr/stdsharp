
#pragma once

#include "../functional/invoke.h"

namespace stdsharp
{
    namespace details
    {
        template<typename T>
        struct function_traits_helper;

        template<typename R, typename... Args, bool Noexcept>
        struct function_traits_helper<R (*)(Args...) noexcept(Noexcept)>
        {
            static auto constexpr is_noexcept = Noexcept;

            using result_t = R;
            using args_t = regular_type_sequence<Args...>;

            using ptr_t = R (*)(Args...) noexcept(Noexcept);

            using function_t = R(Args...) noexcept(Noexcept);
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