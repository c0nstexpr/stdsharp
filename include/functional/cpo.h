//
// Created by BlurringShadow on 2021-9-22.
//
#pragma once
#include "functional/invocable_obj.h"

namespace stdsharp::functional
{
    inline constexpr struct cpo_fn
    {
        template<typename Tag, typename... Args, ::std ::invocable<Tag, Args...> T>
        constexpr decltype(auto) operator()(Tag&& tag, T&& t, Args&&... args) const
            noexcept(concepts ::nothrow_invocable<T, Tag, Args...>)
        {
            return ::std ::invoke(
                ::std ::forward<T>(t),
                ::std ::forward<Tag>(tag),
                ::std ::forward<Args>(args)... //
            );
        }
    } cpo{};

    template<typename Tag, typename... Args>
    concept cpo_invocable = ::std ::invocable<cpo_fn, Tag, Args...>;

    template<typename Tag, typename... Args>
    concept cpo_nothrow_invocable = concepts::nothrow_invocable<cpo_fn, Tag, Args...>;
}