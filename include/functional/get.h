//
// Created by BlurringShadow on 2021-10-15.
//
#pragma once
#include "functional/cpo.h"
#include "functional/functional.h"
#include "functional/get.h"
#include "range/v3/functional/pipeable.hpp"
#include "type_traits/type_traits.h"
#include <type_traits>

// TODO: MSVC ADL bug
// move definition to "functional" namespace
namespace stdsharp
{
    template<::std::size_t N, typename T>
        requires requires { get<N>(::std::declval<T>()); }
    constexpr decltype(auto) adl_get(T&& t) noexcept(noexcept(get<N>(::std::declval<T>())))
    {
        return get<N>(::std::forward<T>(t));
    }
}

namespace stdsharp::functional
{
    namespace details
    {
        template<::std::size_t N>
        struct get_fn
        {
            template<typename T>
                requires requires { adl_get<N>(::std::declval<T>()); }
            constexpr decltype(auto) operator()(T&& t) const
                noexcept(noexcept(adl_get<N>(::std::declval<T>())))
            {
                return adl_get<N>(::std::forward<T>(t));
            }
        };
    }

    template<::std::size_t N>
    struct pack_get_fn
    {
        template<typename... Args>
            requires(N < sizeof...(Args))
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const noexcept
        {
            return ::std::get<N>(::std::tuple<Args&&...>(args...));
        }
    };

    template<::std::size_t N>
    inline constexpr pack_get_fn<N> pack_get{};

    template<::std::size_t N>
    struct get_fn
    {
        template<typename T>
            requires(::std::invocable<details::get_fn<N>, T> && !cpo_invocable<get_fn<N>, T>)
        [[nodiscard]] constexpr decltype(auto) operator()(T&& t) const
            noexcept(concepts::nothrow_invocable<details::get_fn<N>, T>)
        {
            return details::get_fn<N>{}(::std::forward<T>(t));
        }

        template<typename T>
            requires cpo_invocable<get_fn<N>, T>
        [[nodiscard]] constexpr decltype(auto) operator()(T&& t) const
            noexcept(cpo_nothrow_invocable<get_fn<N>, T>)
        {
            return cpo(*this, ::std::forward<T>(t));
        }
    };

    template<::std::size_t N>
    inline constexpr get_fn<N> get{};

    template<::std::size_t N, typename T>
    using get_t = ::std::invoke_result_t<get_fn<N>, T>;

    template<typename T>
    using type_size_seq_t = ::std::make_index_sequence<::std::tuple_size_v<::std::decay_t<T>>>;

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

    inline constexpr struct to_decompose_fn : ::ranges::pipeable_base
    {
        template<typename T>
        class bind_t
        {
            using value_t = type_traits::coerce_t<T>;

            value_t t_;

            template<concepts::decay_same_as<bind_t> This, typename... Args>
                requires ::std::invocable<bind_t, Args...>
            friend decltype(auto) operator|(This&& instance, Args&&... args) //
                noexcept(concepts::nothrow_invocable<bind_t, Args...>)
            {
                return ::std::forward<bind_t>(instance)(std::forward<Args>(args)...);
            }

        public:
            // NOLINTNEXTLINE(hicpp-explicit-conversions)
            constexpr bind_t(T&& t) noexcept(concepts::coercable<T>): t_(t) {}

#define BS_OPERATOR(const_, ref)                                                              \
    template<typename... Args>                                                                \
        requires ::std::invocable<decompose_fn, const_ value_t ref, Args...>                  \
    constexpr decltype(auto) operator()(Args&&... args)                                       \
        const_ ref noexcept(concepts::nothrow_invocable<decompose_fn, T, Args...>)            \
    {                                                                                         \
        return decompose(static_cast<const_ value_t ref>(t_), ::std::forward<Args>(args)...); \
    }
            BS_OPERATOR(, &)
            BS_OPERATOR(const, &)
            BS_OPERATOR(, &&)
            BS_OPERATOR(const, &&)

#undef BS_OPERATOR
        };


        template<typename T>
            requires ::std::constructible_from<bind_t<T>, T>
        [[nodiscard]] constexpr bind_t<T> operator()(T&& t) const
            noexcept(concepts::nothrow_constructible_from<bind_t<T>, T>)
        {
            return t;
        }
    } to_decompose{};
}
