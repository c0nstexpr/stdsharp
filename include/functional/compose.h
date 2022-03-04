#pragma once
#include "utility/value_wrapper.h"
#include "type_traits/core_traits.h"
#include "functional/cpo.h"

namespace stdsharp::functional
{
    namespace details
    {
        struct compose_impl
        {
            template<typename Arg>
            constexpr decltype(auto)
                operator()(Arg&& arg, const type_traits::empty_t) const noexcept
            {
                return ::std::forward<Arg>(arg);
            }

            template<typename... Arg, ::std::invocable<Arg...> First, typename... Fn>
                requires ::std::invocable<
                    compose_impl,
                    ::std::invoke_result_t<First, Arg...>,
                    type_traits::empty_t,
                    Fn... // clang-format off
                > // clang-format on
            constexpr decltype(auto) operator()(
                Arg&&... arg,
                const type_traits::empty_t,
                First&& first,
                Fn&&... fn //
            ) const noexcept( //
                concepts::nothrow_invocable<
                    compose_impl,
                    ::std::invoke_result_t<First, Arg...>,
                    type_traits::empty_t,
                    Fn... // clang-format off
                > // clang-format on
            )
            {
                return (*this)(
                    ::std::invoke(::std::forward<First>(first), std::forward<Arg>(arg)...),
                    type_traits::empty,
                    ::std::forward<Fn>(fn)... //
                );
            }
        };
    }

    template<typename... T>
    struct composed : private value_wrapper<T>...
    {
        template<typename... U>
            requires(::std::constructible_from<value_wrapper<T>, U>&&...)
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        constexpr composed(U&&... u): value_wrapper<T>(::std::forward<U>(u))... {}

#define BS_OPERATOR(const_, ref)                                                                 \
    template<typename... Args>                                                                   \
        requires ::std::invocable<details::compose_impl, Args..., type_traits::empty_t, T...>    \
    constexpr decltype(auto) operator()(Args&&... args) const_ ref noexcept(                     \
        concepts::nothrow_invocable<details::compose_impl, Args..., type_traits::empty_t, T...>) \
    {                                                                                            \
        return details::compose_impl{}(                                                          \
            ::std::forward<Args>(args)...,                                                       \
            type_traits::empty,                                                                  \
            static_cast<const_ T ref>(value_wrapper<T>::value)...);                              \
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
            requires(requires {
                composed{::std::declval<T>()...};
            } && !functional::cpo_invocable<make_composed_fn, T...>)
        constexpr auto operator()(T&&... t) const
            noexcept(noexcept(composed{::std::declval<T>()...}))
        {
            return composed{::std::forward<T>(t)...};
        }

        template<typename... Args>
            requires functional::cpo_invocable<make_composed_fn, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return functional::cpo_invoke(*this, ::std::forward<Args>(args)...);
        }
    } make_composed{};

    template<typename... Fn>
    concept composable = ::std::invocable<make_composed_fn, Fn...>;

    template<typename... Fn>
    concept nothrow_composable = concepts::nothrow_invocable<make_composed_fn, Fn...>;
}
