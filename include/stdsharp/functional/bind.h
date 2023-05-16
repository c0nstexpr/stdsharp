#pragma once

#include "invoke.h"

namespace stdsharp
{
    inline constexpr struct bind_fn
    {
    private:
        template<typename T>
        struct arg_wrapper : value_wrapper<T>
        {
            using value_wrapper<T>::value_wrapper;
        };

        template<typename T>
        struct arg_wrapper<T&> : ::std::reference_wrapper<T>
        {
            using ::std::reference_wrapper<T>::reference_wrapper;
        };

        template<>
        struct arg_wrapper<void>
        {
        };

        static constexpr struct
        {
            template<typename T>
                requires ::std::same_as<template_rebind<::std::decay_t<T>, void>, arg_wrapper<void>>
            [[nodiscard]] constexpr decltype(auto) operator()(T&& wrapper) const noexcept
            {
                return cpp_forward(wrapper).get();
            }

            [[nodiscard]] constexpr decltype(auto) operator()(auto&& v) const noexcept
            {
                return cpp_forward(v);
            }
        } extract{};

        template<typename T>
        using extract_t = decltype(extract(::std::declval<T>()));

    public:
        template<typename Func, typename... Args>
        constexpr auto operator()(Func&& func, Args&&... args) const noexcept( //
            noexcept( //
                ::std::bind_front(
                    projected_invoke,
                    cpp_forward(func),
                    extract,
                    arg_wrapper<persist_t<Args&&>>{cpp_forward(args)}...
                )
            )
        )
            requires requires //
        {
            ::std::bind_front(
                projected_invoke,
                cpp_forward(func),
                extract,
                arg_wrapper<persist_t<Args&&>>{cpp_forward(args)}...
            );
        }
        {
            return ::std::bind_front(
                projected_invoke,
                cpp_forward(func),
                extract,
                arg_wrapper<persist_t<Args&&>>{cpp_forward(args)}...
            );
        }
    } bind{};

    template<typename... Args>
    concept bindable = ::std::invocable<bind_fn, Args...>;

    template<typename... Args>
    concept nothrow_bindable = nothrow_invocable<bind_fn, Args...>;
}