//
// Created by BlurringShadow on 2021-9-22.
//
#pragma once
#include "functional/invocable_obj.h"

namespace stdsharp::functional
{
    template<typename Tag, typename T, typename... Args>
    concept tag_invocable = requires
    {
        tag_invoke(::std::declval<Tag>(), ::std::declval<T>(), std::declval<Args>()...);
    };

    template<typename Tag, typename T, typename... Args>
    concept nothrow_tag_invocable = ::stdsharp::functional::tag_invocable<Tag, T, Args...> &&
        noexcept(tag_invoke(::std::declval<Tag>, ::std::declval<T>, std::declval<Args>()...));

    namespace details
    {
        struct customized_invoke
        {
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
        };

        struct tag_invoke_fn
        {
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

        struct default_cpo_invoke
        {
            template<typename Tag, typename T, typename... Args>
                requires ::std::invocable<Tag, T, Args...>
            constexpr decltype(auto) operator()(Tag&& tag, T&& t, Args&&... args) const
                noexcept(::stdsharp::concepts::nothrow_invocable<T, Args...>)
            {
                return ::std::invoke(
                    ::std::forward<Tag>(tag),
                    ::std::forward<T>(t),
                    ::std::forward<Args>(args)... //
                );
            }
        };
    }

    template<typename...>
    struct cpo_invoke :
        ::ranges::overloaded<
            ::stdsharp::functional::details::tag_invoke_fn,
            ::stdsharp::functional::details::customized_invoke,
            ::stdsharp::functional::details::default_cpo_invoke // clang-format off
        > // clang-format on
    {
    };

    inline constexpr struct
    {
    private:
        template<typename Tag, typename... T>
        struct invocation_base :
            ::std::type_identity<::stdsharp::functional::cpo_invoke<Tag&&, T&&...>>
        {
        };

        template<typename Tag, typename... T>
            requires ::std::invocable<typename invocation_base<Tag, T...>::type, Tag, T...>
        struct nothrow_invocation :
            ::std::bool_constant< //
                ::stdsharp::concepts::nothrow_invocable<
                    typename invocation_base<Tag, T...>::type,
                    Tag,
                    T... // clang-format off
                >
            > // clang-format on
        {
            using type = typename invocation_base<Tag, T...>::type;
        };

        template<
            typename Tag,
            typename... T, // clang-format off
            typename Nothrow_ = nothrow_invocation<Tag, T...>
        > // clang-format on
        static constexpr decltype(auto) invoke_impl(Tag&& tag, T&&... t) noexcept(Nothrow_::value)
        {
            return typename Nothrow_::type{}(
                ::std::forward<Tag>(tag),
                ::std::forward<T>(t)... //
            );
        }

    public:
        template<typename... T>
            requires requires { invoke_impl(::std::declval<T>()...); }
        constexpr decltype(auto) operator()(T&&... t) const
            noexcept(noexcept(invoke_impl(::std::forward<T>(t)...)))
        {
            return invoke_impl(::std::forward<T>(t)...);
        }

        template<::stdsharp::functional::nodiscard_func_obj Tag, typename... T>
            requires requires { invoke_impl(::std::declval<Tag>(), ::std::declval<T>()...); }
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
