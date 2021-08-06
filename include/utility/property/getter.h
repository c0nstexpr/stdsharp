// Created by BlurringShadow at 2021-03-11-下午 4:42

#pragma once

#include "utility/functional.h"

namespace blurringshadow::utility::property
{
    template<::std::invocable GetterFn>
    class getter
    {
    public:
        using value_type = ::std::invoke_result_t<GetterFn>;

    private:
        GetterFn fn_{};

    public:
        template<typename... T>
            requires ::std::constructible_from<GetterFn, T...> // clang-format off
        constexpr explicit getter(T&&... t) // clang-format on
            noexcept(::blurringshadow::utility::nothrow_constructible_from<GetterFn, T...>):
            fn_(::std::forward<T>(t)...)
        {
        }

        constexpr value_type operator()() const
            noexcept(::blurringshadow::utility::nothrow_invocable<GetterFn>)
        {
            return ::std::invoke(fn_);
        }

        constexpr value_type operator()() //
            noexcept(::blurringshadow::utility::nothrow_invocable<GetterFn>)
        {
            return ::std::invoke(fn_);
        }
    };

    template<typename T>
    getter(T&& t) -> getter<::std::remove_cvref_t<T>>;

    inline constexpr auto value_getter = [](auto& t) noexcept
    {
        return ::blurringshadow::utility::property::getter{
            ::blurringshadow::utility::bind_ref_front(identity_v, t) //
        };
    };
}
