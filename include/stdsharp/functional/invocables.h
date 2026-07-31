#pragma once

#include "../utility/value_wrapper.h"
#include "invoke.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename Fn>
    struct invocable : value_wrapper<Fn>
    {
    private:
        using m_base = value_wrapper<Fn>;

    public:
        using m_base::m_base;

        template<typename Self, typename... Args>
            requires std::invocable<Fn, Args...>
        constexpr decltype(auto) operator()(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<Fn, Args...>)
        {
            invoke(cpp_forward(self), cpp_forward(args)...);
        }
    };

    template<typename Base, std::size_t I>
    struct invoke_operator

    {
        using m_base = value_wrapper<Fn>;

    public:
        using m_base::m_base;

        template<
            typename Self,
            typename... Args,
            std::invocable<Args...> Func = forward_like_t<Self, Fn>>
            requires(!static_invocable<Fn, Args...>)
        constexpr decltype(auto) operator()(this Self&& self, Args&&... args) //
            noexcept(nothrow_invocable<Func, Args...>)
        {
            return invoke(cpp_forward(self).Base::template get<I>(), cpp_forward(args)...);
        }
    };

    template<typename...>
    struct invocables;

    template<typename... Func, std::size_t... I>
    struct STDSHARP_EBO invocables<std::index_sequence<I...>, Func...> :
        stdsharp::indexed_values<Func...>,
        private invoke_operator<invocables<std::index_sequence<I...>, Func...>, I>...
    {
    public:
        using invocable<Fn>::operator()...;

        template<typename... Args>
            requires(std::constructible_from<invocable<Fn>, Args> && ...)
        constexpr invocables(Args&&... args) //
            noexcept((nothrow_constructible_from<invocable<Fn>, Args> && ...)):
            invocable<Fn>(cpp_forward(args))...
        {
        }
    };

    template<typename... Fn>
    invocables(Fn&&...) -> invocables<std::decay_t<Fn>...>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::invocables<T...>>
    {
        static constexpr auto value = ::stdsharp::invocables<T...>::size();
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::invocables<T...>>
    {
        using type = typename ::stdsharp::invocables<T...>::template type<I>;
    };
}

#include "../compilation_config_out.h"
