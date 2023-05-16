#pragma once

#include "../utility/value_wrapper.h"
#include "../type_traits/core_traits.h"

namespace stdsharp
{
    namespace details
    {
        struct compose_impl
        {
            template<typename Arg>
            constexpr decltype(auto) operator()(Arg&& arg, const empty_t) const noexcept
            {
                return cpp_forward(arg);
            }

            template<typename... Arg, ::std::invocable<Arg...> First, typename... Fn>
                requires ::std::invocable<
                    compose_impl,
                    ::std::invoke_result_t<First, Arg...>,
                    empty_t,
                    Fn... // clang-format off
                > // clang-format on
            constexpr decltype(auto) operator()(
                Arg&&... arg,
                const empty_t,
                First&& first,
                Fn&&... fn
            ) const noexcept( //
                nothrow_invocable<
                    compose_impl,
                    ::std::invoke_result_t<First, Arg...>,
                    empty_t,
                    Fn... // clang-format off
                > // clang-format on
            )
            {
                return (*this)( //
                    ::std::invoke(cpp_forward(first), cpp_forward(arg)...),
                    empty,
                    cpp_forward(fn)...
                );
            }
        };
    }

    template<typename... T>
    struct composed : private value_wrapper<T>...
    {
        template<typename... U>
            requires(::std::constructible_from<value_wrapper<T>, U> && ...)
        constexpr composed(U&&... u): value_wrapper<T>(cpp_forward(u))...
        {
        }

#define BS_OPERATOR(const_, ref)                                                              \
    template<typename... Args>                                                                \
        requires ::std::invocable<details::compose_impl, Args..., empty_t, T...>              \
    constexpr decltype(auto) operator()(Args&&... args)                                       \
        const_ ref noexcept(nothrow_invocable<details::compose_impl, Args..., empty_t, T...>) \
    {                                                                                         \
        return details::compose_impl{}(                                                       \
            cpp_forward(args)...,                                                             \
            empty,                                                                            \
            static_cast<const_ T ref>(value_wrapper<T>::value)...                             \
        );                                                                                    \
    }
        BS_OPERATOR(, &)
        BS_OPERATOR(const, &)
        BS_OPERATOR(, &&)
        BS_OPERATOR(const, &&)

#undef BS_OPERATOR
    };

    template<typename... T>
    composed(T&&...) -> composed<::std::decay_t<T>...>;

    inline constexpr struct make_composed_fn
    {
        template<typename... T>
            requires requires { composed{::std::declval<T>()...}; }
        constexpr auto operator()(T&&... t) const
            noexcept(noexcept(composed{::std::declval<T>()...}))
        {
            return composed{cpp_forward(t)...};
        }
    } make_composed{};

    template<typename... Fn>
    concept composable = ::std::invocable<make_composed_fn, Fn...>;

    template<typename... Fn>
    concept nothrow_composable = nothrow_invocable<make_composed_fn, Fn...>;
}