// Created by BlurringShadow at 2021-03-11-下午 4:30

#pragma once

#include "utility/functional.h"

namespace blurringshadow::utility::property
{
    template<typename SetterFn>
    class setter
    {
        SetterFn fn_{};

    public:
        template<typename... T>
            requires ::std::constructible_from<SetterFn, T...> // clang-format off
        constexpr explicit setter(T&&... t) // clang-format on
            noexcept(::blurringshadow::utility::nothrow_constructible_from<SetterFn, T...>):
            fn_(::std::forward<T>(t)...)
        {
        }

        template<typename... Args>
            requires ::std::invocable<const SetterFn, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(::blurringshadow::utility::nothrow_invocable<const SetterFn, Args...>)
        {
            return ::std::invoke(fn_, ::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::invocable<SetterFn, Args...>
        constexpr decltype(auto) operator()(Args&&... args) //
            noexcept(::blurringshadow::utility::nothrow_invocable<SetterFn, Args...>)
        {
            return ::std::invoke(fn_, ::std::forward<Args>(args)...);
        }

        template<typename T>
        constexpr auto& operator=(T&& t) const noexcept(noexcept(operator()(::std::forward<T>(t))))
        {
            operator()(::std::forward<T>(t));
            return *this;
        }

        template<typename T>
        constexpr auto& operator=(T&& t) noexcept(noexcept(operator()(::std::forward<T>(t))))
        {
            operator()(std::forward<T>(t));
            return *this;
        }
    };

    template<typename T>
    setter(T&& t) -> setter<::std::remove_cvref_t<T>>;

    inline constexpr auto value_setter = [](auto& t) noexcept
    {
        return ::blurringshadow::utility::property::setter{
            ::blurringshadow::utility::bind_ref_front(assign_v, t) //
        };
    };
}
