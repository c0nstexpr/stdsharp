#pragma once

#include "../functional/invocables.h"
#include "../utility/value_wrapper.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename... T>
    class composed;

    template<typename T, typename... U>
    STDSHARP_EBO class composed<T, U...> : value_wrapper<T>, composed<U...>
    {
        using wrapper = value_wrapper<T>;
        using next_composed = composed<U...>;

    public:
        composed() = default;

        template<typename TArg, typename... UArgs>
            requires std::constructible_from<wrapper, TArg> &&
                         std::constructible_from<next_composed, UArgs...>
        constexpr composed(TArg&& t_arg, UArgs&&... u_args) noexcept(
            nothrow_constructible_from<wrapper, TArg> &&
            nothrow_constructible_from<next_composed, UArgs...>
        ):
            wrapper(cpp_forward(t_arg)), next_composed(cpp_forward(u_args)...)
        {
        }

        template<typename Self, typename... Args>
            requires std::invocable<next_composed, Args...> &&
            std::invocable<std::invoke_result_t<next_composed, Args...>, T>
        constexpr decltype(auto) operator()(this Self&& self, Args&&... args) noexcept(
            nothrow_invocable<next_composed, Args...> &&
            nothrow_invocable<std::invoke_result_t<next_composed, Args...>, T>
        )
        {
            return invoke(
                cpp_forward(self).wrapper::get(),
                cpp_forward(self).next_composed::operator()(cpp_forward(args)...)
            );
        }
    };

    template<typename T>
    class composed<T> : invocables<T>
    {
    };

    template<typename... T>
    composed(T&&...) -> composed<std::decay_t<T>...>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::composed<T...>> : ::std::tuple_size<::std::tuple<T...>>
    {
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::composed<T...>> :
        ::std::tuple_element<I, ::std::tuple<T...>>
    {
    };
}

#include "../compilation_config_out.h"
