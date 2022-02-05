//
// Created by BlurringShadow on 2021-9-22.
//
#pragma once
#include "concepts/concepts.h"

namespace stdsharp::functional
{
    inline constexpr struct cpo_fn
    {
        template<
            typename Tag,
            typename T,
            typename... Args,
            ::std::invocable<Args...> Op =
                decltype(::std::declval<Tag>() << ::std::declval<T>()) // clang-format off
        > // clang-format on
            requires(!::std ::invocable<Tag, Args...>)
        constexpr decltype(auto) operator()(Tag&& tag, T&& t, Args&&... args) const noexcept(
            (noexcept(::std::declval<Tag>() << ::std::declval<T>())) &&
            concepts::nothrow_invocable<Op, Args...> //
        )
        {
            return ::std::invoke(
                ::std::forward<Tag>(tag) << ::std::forward<T>(t),
                ::std::forward<Args>(args)... //
            );
        }

        template<typename Tag, typename... Args, ::std ::invocable<Tag, Args...> T>
        constexpr decltype(auto) operator()(Tag&& tag, T&& t, Args&&... args) const
            noexcept(concepts::nothrow_invocable<T, Tag, Args...>)
        {
            return ::std::invoke(
                ::std::forward<T>(t),
                ::std::forward<Tag>(tag),
                ::std::forward<Args>(args)... //
            );
        }
    } cpo{};

    template<typename... Args>
    concept cpo_invocable = ::std ::invocable<cpo_fn, Args...>;

    template<typename... Args>
    concept cpo_nothrow_invocable = concepts::nothrow_invocable<cpo_fn, Args...>;
}