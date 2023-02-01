#pragma once

#include "type_traits/core_traits.h"
#include "type_traits/member.h"
#include "cassert/cassert.h"

namespace stdsharp
{
    namespace details
    {
        template<typename T>
        struct dispatch_traits
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

                template<typename Storage>
                using type = typename func_traits::result_t (*)(Storage, Args...) //
                    noexcept(func_traits::is_noexcept);
            };

            using type = impl<>;
        };

        template<typename T>
            requires requires { member_function_traits<T>{}; }
        using dispatch_traits_t = typename dispatch_traits<::std::decay_t<T>>::type;

        template<typename ErasedT, typename... Func>
            requires requires { (dispatch_traits_t<Func>{}, ...); }
        class erasure
        {
            template<typename... Traits>
            class impl
            {
                indexed_values<typename Traits::template type<
                    typename Traits::template qualified_t<ErasedT>>...>
                    dispatchers_;

            public:
                template<::std::size_t I>
                using func_traits =
                    function_traits<::std::tuple_element_t<I, decltype(dispatchers_)>>;

                template<typename... T>
                    requires(::std::same_as<nullptr_t, T> || ...)
                constexpr impl(const T...) = delete;

                constexpr impl(const typename Traits:: //
                               template type<typename Traits::template qualified_t<ErasedT>>... t):
                    dispatchers_(t...)
                {
                    debug_assert<::std::invalid_argument>(
                        [&]() noexcept { return ((t == nullptr) || ...); },
                        "one of the arguments is null pointer"
                    );
                }

                template<
                    ::std ::size_t I,
                    typename Ret = typename func_traits<I>::result_t,
                    typename... Args>
                static constexpr auto invocable_r_at =
                    invocable_r<Ret, const typename func_traits<I>::ptr_t&, Args...>;

                template<
                    ::std ::size_t I,
                    typename Ret = typename func_traits<I>::result_t,
                    typename... Args>
                static constexpr auto nothrow_invocable_r_at =
                    nothrow_invocable_r<Ret, const typename func_traits<I>::ptr_t&, Args...>;

                template<
                    ::std ::size_t I,
                    typename Ret = typename func_traits<I>::result_t,
                    typename... Args>
                    requires invocable_r_at<I, Ret, Args...>
                constexpr Ret invoke_at(Args&&... args) const
                    noexcept(nothrow_invocable_r_at<I, Ret, Args...>)
                {
                    return invoke_r<Ret>(get<I>(dispatchers_), ::std ::forward<Args>(args)...);
                }
            };

            using type = impl<dispatch_traits_t<Func>...>;
        };
    }
}