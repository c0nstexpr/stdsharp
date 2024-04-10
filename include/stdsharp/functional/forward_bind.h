#pragma once

#include "../type_traits/indexed_traits.h"
#include "invoke.h"

namespace stdsharp::details
{
    struct forward_bind
    {
        template<typename T>
        static constexpr std::reference_wrapper<T> forward(T& t) noexcept
        {
            return t;
        }

        template<typename T, std::constructible_from<T> U = std::remove_cvref_t<T>>
            requires std::move_constructible<U>
        static constexpr auto forward(T&& t)
            noexcept(nothrow_constructible_from<U, T> && nothrow_move_constructible<U>)
        {
            return U{cpp_forward(t)};
        }

        template<typename T>
        using forward_t = decltype(forward(std::declval<T>()));

        template<typename T, typename ForwardT>
        static constexpr decltype(auto) get_forwarded(ForwardT&& t) noexcept
        {
            if constexpr(std::same_as<T, std::remove_cvref_t<ForwardT>>) return cpp_forward(t);
            else return cpp_forward(t).get();
        }

        template<typename T, typename ForwardT>
        using get_forwarded_t = decltype(get_forwarded<T>(std::declval<ForwardT>()));

        template<typename T, typename Self>
        using get_from_t = get_forwarded_t<T, forward_cast_t<Self, forward_t<T>>>;

        template<typename Impl, typename Func, typename... BindArgs>
        struct binder
        {
            using func_t = std::decay_t<Func>;
            using args_t = stdsharp::indexed_values<forward_t<BindArgs>...>;

            func_t func;
            args_t args;

            template<
                typename Self,
                typename Seq = args_t::index_sequence,
                auto ForwardCast = forward_cast<Self, binder, Impl>>
            constexpr auto operator()(this Self&& self, auto&&... args)
                noexcept(noexcept(ForwardCast(self).invoke(Seq{}, cpp_forward(args)...))) //
                -> decltype(ForwardCast(self).invoke(Seq{}, cpp_forward(args)...))
            {
                return ForwardCast(self).invoke(Seq{}, cpp_forward(args)...);
            }
        };
    };

    template<template<typename...> typename BinderImpl>
    struct forward_bind_fn : forward_bind
    {
        template<
            typename Func,
            typename... BindArgs,
            typename Binder = BinderImpl<Func, BindArgs...>>
        constexpr auto operator()(Func&& func, BindArgs&&... bind_args) const noexcept(
            nothrow_constructible_from<Binder, Func, forward_t<BindArgs>...> &&
            noexcept(Binder{cpp_move(func), {forward(cpp_forward(bind_args))...}})
        )
            requires requires { Binder{cpp_move(func), {forward(cpp_forward(bind_args))...}}; }
        {
            return Binder{cpp_move(func), {forward(cpp_forward(bind_args))...}};
        }
    };

    template<typename Func, typename... BindArgs>
    struct forward_bind_front_binder :
        details::forward_bind::
            binder<forward_bind_front_binder<Func, BindArgs...>, Func, BindArgs...>,
        forward_bind
    {
    private:
        using m_base = details::forward_bind::
            binder<forward_bind_front_binder<Func, BindArgs...>, Func, BindArgs...>;
        friend m_base;

        template<
            typename Self,
            std::size_t... I,
            typename... Args,
            std::invocable<get_from_t<BindArgs, Self>..., Args...> ForwardedFunc =
                forward_cast_t<Self, typename m_base::func_t>>
        constexpr decltype(auto) invoke(
            this Self&& self,
            const std::index_sequence<I...> /*unused*/,
            Args&&... args //
        ) noexcept(nothrow_invocable<ForwardedFunc, get_from_t<BindArgs, Self>..., Args...>)
        {
            return stdsharp::invoke(
                cpp_forward(self).m_base::func,
                get_forwarded<BindArgs>(cpp_forward(self).m_base::args.template get<I>())...,
                cpp_forward(args)...
            );
        }
    };

    template<typename Func, typename... BindArgs>
    struct forward_bind_back_binder :
        details::forward_bind::
            binder<forward_bind_back_binder<Func, BindArgs...>, Func, BindArgs...>,
        forward_bind
    {
    private:
        using m_base = details::forward_bind::
            binder<forward_bind_back_binder<Func, BindArgs...>, Func, BindArgs...>;
        friend m_base;

        template<
            typename Self,
            std::size_t... I,
            typename... Args,
            std::invocable<Args..., get_from_t<BindArgs, Self>...> ForwardedFunc =
                forward_cast_t<Self, typename m_base::func_t>>
        constexpr decltype(auto) invoke(
            this Self&& self,
            const std::index_sequence<I...> /*unused*/,
            Args&&... args //
        ) noexcept(nothrow_invocable<ForwardedFunc, Args..., get_from_t<BindArgs, Self>...>)
        {
            return stdsharp::invoke(
                cpp_forward(self).m_base::func,
                cpp_forward(args)...,
                get_forwarded<BindArgs>(cpp_forward(self).m_base::args.template get<I>())...
            );
        }
    };
}

namespace stdsharp
{
    using forward_bind_front_fn = details::forward_bind_fn<details::forward_bind_front_binder>;

    inline constexpr forward_bind_front_fn forward_bind_front{};

    using forward_bind_back_fn = details::forward_bind_fn<details::forward_bind_back_binder>;

    inline constexpr forward_bind_back_fn forward_bind_back{};
}