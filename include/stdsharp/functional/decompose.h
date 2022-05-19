//
// Created by BlurringShadow on 2021-10-15.
//
#pragma once
#include "bind.h"
#include "pipeable.h"
#include "../tuple/tuple.h"

namespace stdsharp::functional
{
    inline constexpr struct decompose_fn
    {
        template<typename T, ::std::size_t... I, ::std::invocable<get_t<I, T>...> Fn>
        constexpr auto operator()(
            T&& t,
            Fn&& fn,
            const ::std::index_sequence<I...> //
        ) const noexcept(concepts::nothrow_invocable<Fn, get_t<I, T>...>) -> decltype(auto)
        {
            return ::std::invoke(::std::forward<Fn>(fn), get<I>(::std::forward<T>(t))...);
        }

        template<typename T, typename Fn>
            requires ::std::invocable<decompose_fn, T, Fn, type_size_seq_t<T>>
        constexpr auto operator()(T&& t, Fn&& fn) const
            noexcept(concepts::nothrow_invocable<decompose_fn, T, Fn, type_size_seq_t<T>>)
                -> decltype(auto)
        {
            return (*this)(::std::forward<T>(t), ::std::forward<Fn>(fn), type_size_seq_t<T>{});
        }
    } decompose{};

    template<typename... Args>
    concept decomposable = ::std::invocable<decompose_fn, Args...>;

    template<typename... Args>
    concept nothrow_decomposable = concepts::nothrow_invocable<decompose_fn, Args...>;

    inline constexpr struct to_decompose_fn : pipeable_base<>
    {
    private:
        using make_pipeable_fn = make_pipeable_fn<pipe_mode::right>;

        template<typename T>
        using bind_t = bind_type<decompose_fn, T>;

    public:
        template<typename T>
        // requires bindable<decompose_fn, T> && ::std::invocable<make_pipeable_fn, bind_t<T>>
        [[nodiscard]] constexpr auto operator()(T&& t) const noexcept( //
            nothrow_bindable<decompose_fn, T>&&
                concepts::nothrow_invocable<make_pipeable_fn, bind_t<T>> //
        )
        {
            return make_pipeable<pipe_mode::right>(bind(decompose, ::std::forward<T>(t)));
        }
    } to_decompose{};
}
