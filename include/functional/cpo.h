//
// Created by BlurringShadow on 2021-9-22.
//
#pragma once
#include "functional/invocable_obj.h"

namespace stdsharp::functional
{
    template<typename...>
    struct cpo_invoke_fn;

    template<typename Tag, typename T, typename... Args>
    concept tag_invocable = requires
    {
        tag_invoke(::std::declval<Tag>, ::std::declval<T>, std::declval<Args>()...);
    };

    template<typename Tag, typename T, typename... Args>
    concept nothrow_tag_invocable = ::stdsharp::functional::tag_invocable<Tag, T, Args...> &&
        noexcept(tag_invoke(::std::declval<Tag>, ::std::declval<T>, std::declval<Args>()...));

    template<>
    struct cpo_invoke_fn<>
    {
        template<typename Tag, typename T, typename... Args>
            requires(
                ::std::invocable<Tag, T, Args...> &&
                !(::std::invocable<T, Tag, Args...> ||
                  ::stdsharp::functional::tag_invocable<Tag, T, Args...>) //
            )
        constexpr decltype(auto) operator()(Tag&& tag, T&& t, Args&&... args) const
            noexcept(::stdsharp::concepts::nothrow_invocable<T, Args...>)
        {
            return ::std::invoke(
                ::std::forward<Tag>(tag),
                ::std::forward<T>(t),
                ::std::forward<Args>(args)... //
            );
        }

        template<typename Tag, typename T, typename... Args>
            requires ::std::invocable<T, Tag, Args...>
        constexpr decltype(auto) operator()(Tag&& tag, T&& t, Args&&... args) const
            noexcept(::stdsharp::concepts::nothrow_invocable<T, Tag, Args...>)
        {
            return ::std::invoke(
                ::std::forward<T>(t),
                ::std::forward<Tag>(tag),
                ::std::forward<Args>(args)... //
            );
        }

        template<typename Tag, typename T, typename... Args>
            requires ::stdsharp::functional::tag_invocable<Tag, T, Args...>
        constexpr decltype(auto) operator()(Tag&& tag, T&& t, Args&&... args) const
            noexcept(::stdsharp::functional::nothrow_tag_invocable<Tag, T, Args...>)
        {
            return tag_invoke(
                ::std::forward<Tag>(tag),
                ::std::forward<T>(t),
                ::std::forward<Args>(args)... //
            );
        }
    };

    inline constexpr struct
    {
    private:
        template<typename Tag, typename... T>
        struct invocation_base :
            ::std::type_identity<::stdsharp::functional::cpo_invoke_fn<Tag&&, T&&...>>
        {
        };

        template<typename Tag, typename... T>
            requires ::std::invocable<typename invocation_base<Tag, T...>::type, Tag, T...>
        struct nothrow_invocation :
            invocation_base<Tag, T...>,
            ::std::bool_constant< //
                ::stdsharp::concepts::nothrow_invocable<
                    typename invocation_base<Tag, T...>::type,
                    Tag,
                    T... // clang-format off
                >
            > // clang-format on
        {
        };

        template<
            typename Tag,
            typename... T, // clang-format off
            nothrow_invocation<Tag, T...>* Nothrow_ = nullptr
        > // clang-format on
        static constexpr decltype(auto) invoke_impl(Tag&& tag, T&&... t) noexcept(Nothrow_->value)
        {
            return typename decltype(*Nothrow_)::type{}(
                ::std::forward<Tag>(tag),
                ::std::forward<T>(t)... //
            );
        }

    public:
        template<typename... T>
        constexpr decltype(auto) operator()(T&&... t) const
            noexcept(noexcept(invoke_impl(::std::forward<T>(t)...)))
        {
            return invoke_impl(::std::forward<T>(t)...);
        }

        template<::stdsharp::functional::nodiscard_func_obj Tag, typename... T>
        [[nodiscard]] constexpr decltype(auto) operator()(Tag&& tag, T&&... t) const
            noexcept(noexcept(invoke_impl(::std::forward<Tag>(tag), ::std::forward<T>(t)...)))
        {
            return invoke_impl(::std::forward<Tag>(tag), ::std::forward<T>(t)...);
        }
    } cpo{};

    template<typename Tag>
        requires requires { ::std::bool_constant<(Tag{}, true)>{}; }
    inline constexpr auto tagged_cpo = ::std::bind_front(::stdsharp::functional::cpo, Tag{});
}
