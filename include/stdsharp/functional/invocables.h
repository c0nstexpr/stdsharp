//
// Created by BlurringShadow on 2021-10-15.
//
#pragma once

#include "../type_traits/core_traits.h"
#include "../utility/value_wrapper.h"

namespace stdsharp::functional
{
    namespace details
    {
        template<typename... Func>
        struct trivial
        {
            template<typename... Args>
            static constexpr auto value = []
            {
                constexpr auto size = sizeof...(Func);
                ::std::size_t i = 0;
                ::std::size_t target = size;

                for(const auto v : {::std::invocable<Func, Args...>...})
                {
                    if(v)
                    {
                        if(target == size) target = i;
                        else
                            return size;
                    }

                    ++i;
                }
                return target;
            }();
        };

        template<typename... Func>
        struct sequenced
        {
            template<typename... Args>
            static constexpr auto value = []
            {
                ::std::size_t i = 0;
                for(const auto v : {::std::invocable<Func, Args...>...})
                {
                    if(v) break;
                    ++i;
                }
                return i;
            }();
        };

        template<::std::size_t I, typename Func>
        struct indexed_func : value_wrapper<Func>
        {
            using m_base = value_wrapper<Func>;
            using m_base::m_base;

#define STDSHARP_GET(const_, ref)                                                          \
    template<typename... Args>                                                             \
    constexpr decltype(auto) get(const type_traits::index_constant<I>) const_ ref noexcept \
    {                                                                                      \
        return static_cast<const_ Func ref>(static_cast<const_ m_base ref>(*this));        \
    }

            STDSHARP_GET(, &)
            STDSHARP_GET(const, &)
            STDSHARP_GET(, &&)
            STDSHARP_GET(const, &&)

#undef STDSHARP_GET
        };

        template<template<typename...> typename, typename, typename...>
        class invocables;

        template<template<typename...> typename Selector, ::std::size_t... I, typename... Func>
        class invocables<Selector, ::std::index_sequence<I...>, Func...> : indexed_func<I, Func>...
        {
        private:
            using indexed_func<I, Func>::get...;

            template<::std::size_t Index, concepts::decay_same_as<invocables> This>
            friend constexpr decltype(auto) get(This&& this_) noexcept
            {
                return ::std::forward<This>(this_).get(type_traits::index_constant<Index>{});
            }

        public:
            invocables() = default;

            template<typename... Args>
                requires(::std::constructible_from<value_wrapper<Func>, Args>&&...)
            constexpr invocables(Args&&... args) //
                noexcept((concepts::nothrow_constructible_from<value_wrapper<Func>, Args> && ...)):
                indexed_func<I, Func>(::std::forward<Args>(args))...
            {
            }

            template<::std::size_t Index, typename... Args, typename This>
                requires(Index < sizeof...(Func))
            static constexpr decltype(auto) invoke_impl(This&& this_, Args&&... args) noexcept( //
                noexcept( //
                    concepts::nothrow_invocable<
                        decltype(::std::declval<This>().get(type_traits::index_constant<Index>{})),
                        Args... // clang-format off
                    >
                ) // clang-format on
            )
            {
                return ::std::invoke(
                    ::std::forward<This>(this_).get(type_traits::index_constant<Index>{}),
                    ::std::forward<Args>(args)... //
                );
            }

#define BS_OPERATOR(const_, ref)                                                            \
    template<                                                                               \
        typename... Args,                                                                   \
        typename This = const_ invocables ref,                                              \
        auto Index = Selector<const_ Func ref...>::template value<Args...>>                 \
        requires requires                                                                   \
        {                                                                                   \
            invoke_impl<Index>(::std::declval<This>(), ::std::declval<Args>()...);          \
        }                                                                                   \
    constexpr decltype(auto) operator()(Args&&... args) const_ ref noexcept(                \
        noexcept(invoke_impl<Index>(::std::declval<This>(), ::std::declval<Args>()...)))    \
    {                                                                                       \
        return invoke_impl<Index>(static_cast<This>(*this), ::std::forward<Args>(args)...); \
    }

            BS_OPERATOR(, &)
            BS_OPERATOR(const, &)
            BS_OPERATOR(, &&)
            BS_OPERATOR(const, &&)
#undef BS_OPERATOR
        };
    }

    template<typename... Func>
    struct trivial_invocables :
        details::invocables<details::trivial, ::std::index_sequence_for<Func...>, Func...>
    {
        using base =
            details::invocables<details::trivial, ::std::index_sequence_for<Func...>, Func...>;

    public:
        using base::base;
        using base::operator();
    };

    template<typename... Func>
    trivial_invocables(Func&&...) -> trivial_invocables<::std::decay_t<Func>...>;

    template<typename... Func>
    struct sequenced_invocables :
        details::invocables<details::sequenced, ::std::index_sequence_for<Func...>, Func...>
    {
        using base =
            details::invocables<details::sequenced, ::std::index_sequence_for<Func...>, Func...>;

    public:
        using base::base;
        using base::operator();
    };

    template<typename... Func>
    sequenced_invocables(Func&&...) -> sequenced_invocables<::std::decay_t<Func>...>;

    template<typename Func>
    class nodiscard_invocable : value_wrapper<Func>
    {
        using base = value_wrapper<Func>;

    public:
        using base::base;

#define BS_OPERATOR(const_, ref)                                                       \
    template<typename... Args>                                                         \
        requires ::std::invocable<const_ Func ref, Args...>                            \
    [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args)                  \
        const_ ref noexcept(concepts::nothrow_invocable<const_ Func ref, Args...>)     \
    {                                                                                  \
        return ::std::invoke(                                                          \
            static_cast<const_ Func ref>(base::value), ::std::forward<Args>(args)...); \
    }

        BS_OPERATOR(, &)
        BS_OPERATOR(const, &)
        BS_OPERATOR(, &&)
        BS_OPERATOR(const, &&)
#undef BS_OPERATOR
    };

    template<typename Func>
    nodiscard_invocable(Func&& func) -> nodiscard_invocable<::std::decay_t<Func>>;

    inline constexpr struct make_trivial_invocables_fn
    {
        template<typename... Invocable>
            requires requires { trivial_invocables{::std::declval<Invocable>()...}; }
        [[nodiscard]] constexpr auto operator()(Invocable&&... invocable) const //
            noexcept(noexcept(trivial_invocables{::std::declval<Invocable>()...}))
        {
            return trivial_invocables{::std::forward<Invocable>(invocable)...};
        }
    } make_trivial_invocables{};

    inline constexpr struct make_sequenced_invocables_fn
    {
        template<typename... Invocable>
            requires requires { sequenced_invocables{::std::declval<Invocable>()...}; }
        [[nodiscard]] constexpr auto operator()(Invocable&&... invocable) const //
            noexcept(noexcept(sequenced_invocables{::std::declval<Invocable>()...}))
        {
            return sequenced_invocables{::std::forward<Invocable>(invocable)...};
        }
    } make_sequenced_invocables{};
}
