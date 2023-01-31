#pragma once

#include "stdsharp/type_traits/core_traits.h"
#include "type_traits/member.h"

namespace stdsharp
{
    namespace details
    {
        template<typename T>
        struct erasure_dispatch_traits
        {
            using func_traits = member_function_traits<T>;

            template<typename = typename func_traits::args_t>
            struct impl;

            template<template<typename...> typename Seq, typename... Args>
            struct impl<Seq<Args...>>
            {
                template<typename U>
                using qualified_t = apply_qualifiers<
                    U,
                    func_traits::is_const,
                    func_traits::is_volatile,
                    func_traits::ref_type // clang-format off
                >; // clang-format on

                template<typename First>
                using type = typename func_traits::result_t (*)(qualified_t<First>, Args...) //
                    noexcept(func_traits::is_noexcept);

                template<typename First>
                using func_traits = function_traits<First>;
            };

            using type = impl<>;
        };
    }

    template<typename T>
        requires requires { member_function_traits<T>{}; }
    using erasure_dispatch_traits =
        typename details::erasure_dispatch_traits<::std::decay_t<T>>::type;

    template<typename Storage, typename... Func>
        requires requires { (erasure_dispatch_traits<Func>{}, ...); }
    class ref_erasure
    {
        static constexpr indexed_types<erasure_dispatch_traits<Func>...> dispatcher_traits;

        indexed_values<typename erasure_dispatch_traits<Func>::template type<Storage>...>
            dispatchers_;
        Storage storage_;

        template<::std::size_t I>
        using dispatch_traits = ::std::tuple_element_t<I, decltype(dispatcher_traits)>;

        template<::std::size_t I>
        using func_traits = function_traits<::std::tuple_element_t<I, decltype(dispatchers_)>>;

        template<::std::size_t I>
        using qualified_storage = func_traits<I>;

        template<::std::size_t I, typename Ret, typename... Args>
        static constexpr auto invocable_r_at = func_traits<I>::template invocable_r<Ret, Args...>;

        template<::std::size_t I, typename Ret, typename... Args>
        static constexpr auto nothrow_invocable_r_at =
            func_traits<I>::template nothrow_invocable_r<Ret, Args...>;

    public:
        template<typename... T>
            requires(::std::same_as<T, nullptr_t> || ...)
        ref_erasure(T...) = delete;

        template<typename... T>
            requires(::std::constructible_from<Storage, T> && ...)
        constexpr ref_erasure(
            T&&... t,
            const typename function_traits<Func>::ptr_t... funcs
        ) noexcept(nothrow_constructible_from<Storage, T...>):
            storage_(::std::forward<T>(t)...), dispatchers_(funcs...)
        {
        }

        template<
            ::std::size_t I,
            typename Ret = typename func_traits<I>::result_t,
            typename... Args // clang-format off
        > // clang-format on
            requires invocable_r_at<I, Ret, Args...>
        constexpr Ret invoke_at(Args&&... args) noexcept(nothrow_invocable_r_at<I, Ret, Args...>)
        {
            return func_traits<I>::template invoke_r<Ret>(
                get<I>(dispatchers_),
                static_cast<>(storage_),
                ::std::forward<Args>(args)...
            );
        }

        template<
            ::std::size_t I,
            typename... Args,
            typename Ret = typename func_traits<I>::result_t // clang-format off
        > // clang-format on
            requires invocable_r_at<I, Ret, Args...>
        constexpr decltype(auto) operator()(Args&&... args) //
            noexcept(nothrow_invocable_r_at<I, Ret, Args...>)
        {
            return invoke_at<I>(::std::forward<Args>(args)...);
        }
    };
}