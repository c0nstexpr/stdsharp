//
// Created by BlurringShadow on 2021-9-22.
//

#include "functional/functional.h"

namespace stdsharp::functional
{
    template<typename...>
    struct cpo_invoke_fn
    {
        template<typename Tag, typename T, typename... Args>
            requires ::std::invocable<T, Args...>
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
            requires requires
            {
                tag_invoke(::std::declval<Tag>, ::std::declval<T>, std::declval<Args>()...);
            }
        constexpr decltype(auto) invoke_impl(Tag&& tag, T&& t, Args&&... args) const noexcept( //
            noexcept( //
                tag_invoke(
                    ::std::forward<Tag>(tag),
                    ::std::forward<T>(t),
                    ::std::forward<Args>(args)... // clang-format off
                )
            )
        ) // clang-format on
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

        template<typename Tag, typename... T>
            requires(
                ::std::derived_from<Tag, ::stdsharp::functional::nodiscard_tag_t> ||
                ::std::same_as<decltype(::std::ranges::begin), Tag>)
        [[nodiscard]] constexpr decltype(auto) operator()(Tag&& tag, T&&... t) const
            noexcept(noexcept(invoke_impl(::std::forward<Tag>(tag), ::std::forward<T>(t)...)))
        {
            return invoke_impl(::std::forward<Tag>(tag), ::std::forward<T>(t)...);
        }
    } cpo{};
}
