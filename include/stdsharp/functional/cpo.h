//
// Created by BlurringShadow on 2021-9-22.
//
#pragma once

#include <functional>

#include "../concepts/concepts.h"
#include "../type_traits/core_traits.h"

namespace stdsharp::functional
{
    inline constexpr struct cpo_invoke_fn
    {
        template<
            typename Tag,
            typename T,
            typename... Args,
            ::std::invocable<Args...> Op =
                decltype(::std::declval<Tag>() << ::std::declval<T>()) // clang-format off
        > // clang-format on
            requires(!::std::invocable<Tag, Args...>)
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
    } cpo_invoke{};

    template<typename... Args>
    concept cpo_invocable = ::std ::invocable<cpo_invoke_fn, Args...>;

    template<typename... Args>
    concept cpo_nothrow_invocable = concepts::nothrow_invocable<cpo_invoke_fn, Args...>;

    template<typename CPO, ::std::default_initializable DefaultImpl>
    struct cpo_fn
    {
        template<typename... Args>
            requires requires
            {
                requires !cpo_invocable<CPO, Args...>;
                requires ::std::invocable<DefaultImpl, Args...>;
            }
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(concepts::nothrow_invocable<DefaultImpl, Args...>)
        {
            return DefaultImpl{}(::std ::forward<Args>(args)...);
        }

        template<typename... Args>
            requires cpo_invocable<CPO, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(cpo_nothrow_invocable<CPO, Args...>)
        {
            return cpo_invoke(static_cast<const CPO&>(*this), ::std ::forward<Args>(args)...);
        }
    };
}