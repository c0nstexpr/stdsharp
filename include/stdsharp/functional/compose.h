#pragma once

#include "../functional/invocables.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<std::size_t I = 0, typename Fn>
    static constexpr decltype(auto) composed_invoke(Fn&& fn, auto&&... arg) noexcept( //
        noexcept( //
            composed_invoke<I + 1>(
                cpp_forward(fn),
                invoke_at<I>(cpp_forward(fn), cpp_forward(arg)...)
            )
        )
    )
        requires requires {
            composed_invoke<I + 1>(
                cpp_forward(fn),
                invoke_at<I>(cpp_forward(fn), cpp_forward(arg)...)
            );
        }
    {
        return composed_invoke<I + 1>(
            cpp_forward(fn),
            invoke_at<I>(cpp_forward(fn), cpp_forward(arg)...)
        );
    }

    template<std::size_t I, typename... T>
        requires(I == std::tuple_size_v<invocables<T...>>)
    static constexpr decltype(auto) composed_invoke(
        const invocables<T...>& /*unused*/,
        auto&& arg //
    ) noexcept
    {
        return cpp_forward(arg);
    }
}

namespace stdsharp
{
    template<typename... T>
    class composed : public invocables<T...>
    {
        using invocables = invocables<T...>;

    private:
        static constexpr decltype(auto) operator_impl(auto&&... args) //
            noexcept(noexcept(details::composed_invoke(cpp_forward(args)...)))
            requires requires { details::composed_invoke(cpp_forward(args)...); }
        {
            return compose_invoke(cpp_forward(args)...);
        }

    public:
        using invocables::invocables;

        composed() = default;

        STDSHARP_MEM_PACK(operator(), operator_impl, composed)
    };

    template<typename... T>
    composed(T&&...) -> composed<std::decay_t<T>...>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::composed<T...>> :
        ::std::tuple_size<::stdsharp::indexed_types<T...>>
    {
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::composed<T...>> :
        ::std::tuple_element<I, ::stdsharp::indexed_types<T...>>
    {
    };
}

#include "../compilation_config_out.h"