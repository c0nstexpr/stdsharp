
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

            template<typename Ret, typename... InArgs>
            static constexpr auto invocable_r = stdsharp::invocable_r<ptr_t, Ret, InArgs...>;

            template<typename Ret, typename... InArgs>
            static constexpr auto nothrow_invocable_r =
                stdsharp::nothrow_invocable_r<ptr_t, Ret, InArgs...>;

            template<typename Ret = R, typename... InArgs>
                requires invocable_r<ptr_t, Ret, InArgs...>
            static constexpr Ret invoke_r(const ptr_t ptr, InArgs&&... args) //
                noexcept(nothrow_invocable_r<ptr_t, Ret, InArgs...>)
            {
                return stdsharp::invoke_r<Ret>(ptr, ::std::forward<Args>(args)...);
            }
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