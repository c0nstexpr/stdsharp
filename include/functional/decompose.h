//
// Created by BlurringShadow on 2021-10-15.
//
#pragma once
#include "functional/cpo.h"
#include "functional/functional.h"
#include "type_traits/type_traits.h"
#include "utility/utility.h"
#include <type_traits>

namespace stdsharp::functional
{
    inline constexpr struct decompose_size_t
    {
    } decompose_size{};

    template<::std::size_t I>
    struct decompose_fn
    {
    };

    template<::std::size_t I>
    inline constexpr cpo_fn<decompose_fn<I>> decompose{};

    template<::std::size_t I, typename Decomposer, typename Param>
    using decompose_t = ::std::invoke_result_t<decompose_fn<I>, Decomposer, Param>;

    template<typename Decomposer>
    struct decompose_to_fn : Decomposer
    {
        using Decomposer::Decomposer;

#define BS_DECOMPOSE_TO_FN_OPERATOR(const_, ref_)                                              \
    template<                                                                                  \
        ::std::size_t... I,                                                                    \
        typename Param,                                                                        \
        ::std::invocable<decompose_t<I, Decomposer, Param>...> Fn>                             \
    constexpr decltype(auto) operator()(                                                       \
        Param&& param,                                                                         \
        Fn&& fn,                                                                               \
        const ::std::index_sequence<I...> = ::std::make_index_sequence<Decomposer::size>{})    \
        const_ ref_ noexcept(                                                                  \
            concepts::nothrow_invocable<Fn, decompose_t<I, Decomposer, Param>...>)             \
    {                                                                                          \
        return ::std::invoke(                                                                  \
            ::std::forward<Fn>(fn),                                                            \
            decompose<I>(                                                                      \
                static_cast<const_ Decomposer ref_>(*this), ::std::forward<Param>(param))...); \
    }

        BS_DECOMPOSE_TO_FN_OPERATOR(const, &)
        BS_DECOMPOSE_TO_FN_OPERATOR(const, &&)
        BS_DECOMPOSE_TO_FN_OPERATOR(, &)
        BS_DECOMPOSE_TO_FN_OPERATOR(, &&)
#undef BS_DECOMPOSE_TO_FN_OPERATOR
    };
}
