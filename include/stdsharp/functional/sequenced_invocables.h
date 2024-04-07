#pragma once

#include "../type_traits/indexed_traits.h"
#include "invoke.h"

#include <tuple>

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename...>
    struct sequenced_invocables;

    template<typename... Func, std::size_t... I>
    struct STDSHARP_EBO sequenced_invocables<std::index_sequence<I...>, Func...> :
        indexed_value<I, Func>...
    {
    private:
        template<typename Self, std::size_t J, typename... Args>
            requires requires { requires J < sizeof...(Func); }
        static consteval auto first_invocable() noexcept
        {
            using fn = forward_cast_t<Self, type<J>>;

            if constexpr(std::invocable<fn, Args...>) return J;
            else return first_invocable<Self, J + 1, Args...>();
        }

    public:
        static constexpr auto size() noexcept { return sizeof...(Func); }

        template<std::size_t J>
        using type = type_at<J, Func...>;

        template<std::size_t J, typename Self>
        constexpr forward_cast_t<Self, type<J>> get(this Self&& self) noexcept
        {
            return forward_cast<Self, sequenced_invocables, indexed_value<J, type<J>>>(self).get();
        }

        template<std::size_t J, typename Self, typename SelfT = const Self>
        constexpr forward_cast_t<SelfT, type<J>> cget(this const Self&& self) noexcept
        {
            return forward_cast<SelfT, sequenced_invocables, indexed_value<J, type<J>>>(self) //
                .cget();
        }

        template<std::size_t J, typename Self, typename SelfT = const Self&>
        constexpr forward_cast_t<SelfT, type<J>> cget(this const Self& self) noexcept
        {
            return forward_cast<SelfT, sequenced_invocables, indexed_value<J, type<J>>>(self) //
                .cget();
        }

        template<
            typename Self,
            typename... Args,
            auto J = first_invocable<Self, 0, Args...>(),
            std::invocable<Args...> Fn = forward_cast_t<Self, type<J>>>
        constexpr decltype(auto) operator()(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<Fn, Args...>)
        {
            auto&& fn = forward_cast<Self, sequenced_invocables>(self).template get<J>();
            return invoke(cpp_forward(fn), cpp_forward(args)...);
        }
    };
}

namespace stdsharp
{
    template<typename... Func>
    struct sequenced_invocables :
        details::sequenced_invocables<std::index_sequence_for<Func...>, Func...>
    {
    };

    template<typename... Func>
    sequenced_invocables(Func&&...) -> sequenced_invocables<std::decay_t<Func>...>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::sequenced_invocables<T...>>
    {
        static constexpr auto value = ::stdsharp::sequenced_invocables<T...>::size();
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::sequenced_invocables<T...>>
    {
        using type = ::stdsharp::sequenced_invocables<T...>::template type<I>;
    };
}

#include "../compilation_config_out.h"