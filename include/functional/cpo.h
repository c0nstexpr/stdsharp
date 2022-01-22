//
// Created by BlurringShadow on 2021-9-22.
//
#pragma once
#include "functional/invocable_obj.h"

namespace stdsharp::functional
{
    template<typename Tag, typename... Args>
    concept tag_invocable = requires
    {
        tag_invoke(::std::declval<Tag>(), ::std::declval<Args>()...);
    };

    template<typename Tag, typename... Args>
    concept nothrow_tag_invocable = tag_invocable<Tag, Args...> &&
        noexcept(tag_invoke(::std::declval<Tag>(), ::std::declval<Args>()...));

    namespace details
    {
        template<typename Tag>
        struct tag_invoke_fn : Tag
        {
            template<typename... Args>
                requires tag_invocable<Tag, Args...>
            constexpr decltype(auto) operator()(Args&&... args) const
                noexcept(nothrow_tag_invocable<Tag, Args...>)
            {
                return tag_invoke(static_cast<Tag>(*this), ::std::forward<Args>(args)...);
            }
        };

        template<typename Tag>
        struct customized_operator_invoke : Tag
        {
            template<typename T, typename... Args>
                requires ::std::invocable<T, Tag, Args...>
            constexpr decltype(auto) operator()(T&& t, Args&&... args) const
                noexcept(concepts::nothrow_invocable<T, Tag, Args...>)
            {
                return ::std::invoke(
                    ::std::forward<T>(t),
                    static_cast<Tag>(*this),
                    ::std::forward<Args>(args)... //
                );
            }
        };

        template<typename Tag>
        struct default_cpo_invoke : Tag
        {
            template<typename... Args>
                requires ::std::invocable<Tag, Args...>
            constexpr decltype(auto) operator()(Args&&... args) const
                noexcept(concepts::nothrow_invocable<Tag, Args...>)
            {
                return ::std::invoke(static_cast<Tag>(*this), ::std::forward<Args>(args)...);
            }
        };
    }

    template<typename Tag>
    struct cpo_t :
        ::ranges::overloaded<
            details::tag_invoke_fn<Tag>,
            details::customized_operator_invoke<Tag>,
            details::default_cpo_invoke<Tag> // clang-format off
        > // clang-format on
    {
        using base = ::ranges::overloaded<
            details::tag_invoke_fn<Tag>,
            details::customized_operator_invoke<Tag>,
            details::default_cpo_invoke<Tag> // clang-format off
        >; // clang-format on

        template<typename... T>
            requires ::std::invocable<base, T...>
        constexpr decltype(auto) operator()(T&&... t) const
            noexcept(concepts::nothrow_invocable<base, T...>)
        {
            return base::operator()(::std::forward<T>(t)...);
        }

        template<typename... T>
            requires ::std::invocable<base, T...> && nodiscard_func_obj<Tag>
        [[nodiscard]] constexpr decltype(auto) operator()(T&&... t) const
            noexcept(concepts::nothrow_invocable<base, T...>)
        {
            return base::operator()(::std::forward<T>(t)...);
        }
    };
}