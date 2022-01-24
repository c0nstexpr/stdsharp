//
// Created by BlurringShadow on 2021-10-15.
//
#pragma once
#include "functional/cpo.h"
#include "functional/functional.h"
#include "range/v3/functional/pipeable.hpp"
#include "range/v3/range_fwd.hpp"
#include "type_traits/type_traits.h"
#include <memory>
#include <utility>

namespace stdsharp::functional
{
    template<::std::size_t N>
    struct pack_get_fn : nodiscard_tag_t
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

    namespace details
    {
        template<::std::size_t N>
        struct std_get_fn : nodiscard_tag_t
        {
            template<typename T>
                requires requires { ::std::get<N>(::std::declval<T>()); }
            [[nodiscard]] constexpr decltype(auto) operator()(T&& t) const
                noexcept(noexcept(::std::get<N>(::std::declval<T>())))
            {
                return ::std::get<N>(::std::forward<T>(t));
            }
        };

        template<::std::size_t N>
        struct adl_get_fn : nodiscard_tag_t
        {
            template<typename T>
                requires requires { get<N>(::std::declval<T>()); }
            [[nodiscard]] constexpr decltype(auto) operator()(T&& t) const
                noexcept(noexcept(get<N>(::std::declval<T>())))
            {
                return get<N>(::std::forward<T>(t));
            }
        };
    }

    template<::std::size_t N>
    struct get_fn : ::ranges::overloaded<details::adl_get_fn<N>, details::std_get_fn<N>>
    {
    };

    template<::std::size_t N>
    inline constexpr cpo_fn<get_fn<N>> get{};

    template<::std::size_t N, typename... Args>
    using get_t = decltype(get<N>(::std::declval<Args>()...));

    inline constexpr struct decompose_fn : ::ranges::pipeable_base
    {
        template<typename T>
        class with_param
        {
            T t_;

            template<typename U>
                requires ::std::constructible_from<T, U>
            constexpr explicit with_param(U&& u) noexcept(
                concepts::nothrow_constructible_from<T, U>):
                t_(::std::forward<U>(u))
            {
            }

            friend struct decompose_fn;

            template<::std::size_t... I, typename U, ::std::invocable<get_t<I, U>...> Fn>
            static constexpr decltype(auto) impl(
                Fn&& fn,
                U&& u,
                const ::std::index_sequence<I...> // clang-format off
            ) noexcept(concepts::nothrow_invocable<Fn, get_t<I, T>...>) // clang-format on
            {
                return ::std::invoke(
                    ::std::forward<Fn>(fn),
                    functional::get<I>(::std::forward<U>(u))... //
                );
            }


        public:
#define BS_OPERATOR(const_, ref_)                                                                \
    template<typename Fn, typename Seq = ::std::make_index_sequence<::std::tuple_size_v<T>>>     \
    constexpr decltype(auto) operator()(Fn&& fn, const Seq seq = {})                             \
        const_ ref_ noexcept(noexcept(                                                           \
            impl(::std::declval<Fn>(), ::std::declval<const_ T ref_>(), seq))) requires requires \
    {                                                                                            \
        impl(::std::declval<Fn>(), ::std::declval<const_ T ref_>(), seq);                        \
    } { return impl(::std::forward<Fn>(fn), static_cast<const_ T ref_>(t_), seq); }              \
                                                                                                 \
    template<typename Fn>                                                                        \
        requires ::std::invocable<const_ with_param ref_, Fn>                                    \
    constexpr decltype(auto) operator|(Fn&& fn)                                                  \
        const_ ref_ noexcept(concepts::nothrow_invocable<const_ with_param ref_, Fn>)            \
    {                                                                                            \
        return static_cast<const_ with_param ref_>(*this)(::std::forward<Fn>(fn));               \
    }

            BS_OPERATOR(const, &)
            BS_OPERATOR(const, &&)
            BS_OPERATOR(, &)
            BS_OPERATOR(, &&)
#undef BS_OPERATOR
        };

        template<typename T>
        constexpr auto operator()(T&& t) const noexcept
        {
            return with_param<T>{::std::forward<T>(t)};
        }
    } decompose{};
}
