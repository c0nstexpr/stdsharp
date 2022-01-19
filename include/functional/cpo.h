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
        struct customized_operator_invoke
        {
            template<typename Tag, typename T, typename... Args>
                requires ::std::invocable<T, Tag, Args...>
            constexpr decltype(auto) operator()(Tag&& tag, T&& t, Args&&... args) const
                noexcept(concepts::nothrow_invocable<T, Tag, Args...>)
            {
                return ::std::invoke(
                    ::std::forward<T>(t),
                    ::std::forward<Tag>(tag),
                    ::std::forward<Args>(args)... //
                );
            }
        };

        struct tag_invoke_fn
        {
            template<typename Tag, typename... Args>
                requires tag_invocable<Tag, Args...>
            constexpr decltype(auto) operator()(Tag&& tag, Args&&... args) const
                noexcept(nothrow_tag_invocable<Tag, Args...>)
            {
                return tag_invoke(::std::forward<Tag>(tag), ::std::forward<Args>(args)...);
            }
        };

        struct default_cpo_invoke
        {
            template<typename Tag, typename... Args>
                requires ::std::invocable<Tag, Args...>
            constexpr decltype(auto) operator()(Tag&& tag, Args&&... args) const
                noexcept(concepts::nothrow_invocable<Tag, Args...>)
            {
                return ::std::invoke(::std::forward<Tag>(tag), ::std::forward<Args>(args)...);
            }
        };
    }

    template<typename...>
    struct cpo_invoke :
        ::ranges::overloaded<
            details::tag_invoke_fn,
            details::customized_operator_invoke,
            details::default_cpo_invoke // clang-format off
        > // clang-format on
    {
    };

    inline constexpr struct cpo_t
    {
    private:
        template<
            typename Tag,
            typename... T,
            ::std::invocable<Tag, T...> CPOInvoker = cpo_invoke<Tag&&, T&&...> // clang-format off
        > // clang-format on
            requires ::std::default_initializable<CPOInvoker>
        static constexpr decltype(auto) invoke_impl(Tag&& tag, T&&... t) noexcept(
            concepts::nothrow_invocable<CPOInvoker, Tag, T...>&& //
                concepts::nothrow_default_initializable<CPOInvoker> //
        )
        {
            return CPOInvoker{}(::std::forward<Tag>(tag), ::std::forward<T>(t)...);
        }

    public:
        template<typename... T>
            requires requires { invoke_impl(::std::declval<T>()...); }
        constexpr decltype(auto) operator()(T&&... t) const
            noexcept(noexcept(invoke_impl(::std::forward<T>(t)...)))
        {
            return invoke_impl(::std::forward<T>(t)...);
        }

        template<nodiscard_func_obj Tag, typename... T>
            requires requires { invoke_impl(::std::declval<Tag>(), ::std::declval<T>()...); }
        [[nodiscard]] constexpr decltype(auto) operator()(Tag&& tag, T&&... t) const
            noexcept(noexcept(invoke_impl(::std::forward<Tag>(tag), ::std::forward<T>(t)...)))
        {
            return invoke_impl(::std::forward<Tag>(tag), ::std::forward<T>(t)...);
        }
    } cpo{};

    template<typename Tag>
    struct tagged_cpo_t : Tag
    {
        using Tag::Tag;

        template<typename... Args>
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(concepts::nothrow_invocable<cpo_t, Tag, Args...>)
        {
            return cpo(static_cast<Tag>(*this), ::std::forward<Args>(args)...);
        }
    };

    template<typename Tag>
        requires nodiscard_func_obj<Tag>
    struct tagged_cpo_t<Tag> : Tag, nodiscard_tag_t
    {
        using Tag::Tag;

        template<typename... Args>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(concepts::nothrow_invocable<cpo_t, Tag, Args...>)
        {
            return cpo(static_cast<Tag>(*this), ::std::forward<Args>(args)...);
        }
    };
}
