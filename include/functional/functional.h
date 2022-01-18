#pragma once
#include <range/v3/functional.hpp>
#include <type_traits>
#include <utility>

#include "functional/operations.h"
#include "utility/utility.h"

namespace stdsharp::functional
{
    inline constexpr auto empty_invoke = [](const auto&...) noexcept
    {
        return type_traits::empty; //
    };

    inline constexpr auto projected_invoke = []<typename Fn, typename Projector, typename... Args>
        requires(
            (::std::invocable<Projector&, Args> && ...) &&
            ::std::invocable<Fn, ::std::invoke_result_t<Projector, Args>...> //
        )
    ( //
        Fn&& fn,
        Projector projector,
        Args&&... args // clang-format off
    ) noexcept(concepts::nothrow_invocable<Fn, ::std::invoke_result_t<Projector, Args>...>) // clang-format on
        ->decltype(auto)
    {
        return ::std::invoke(
            ::std::forward<Fn>(fn), //
            ::std::invoke(projector, ::std::forward<Args>(args))... //
        );
    };

    template<typename ReturnT>
    inline constexpr invocable_obj invoke_r(
        nodiscard_tag,
        []<typename Func, typename... Args>(Func&& func, Args&&... args) // clang-format off
            noexcept(concepts::nothrow_invocable_r<Func, ReturnT, Args...>)
            -> ::std::enable_if_t<concepts::invocable_r<Func, ReturnT, Args...>, ReturnT>
        {
            return static_cast<ReturnT>(
                ::std::invoke(::std::forward<Func>(func), ::std::forward<Args>(args)...)
            );
        } // clang-format on
    );

    namespace details
    {
        template<typename Func, typename... T>
        class bind_ref_front_invoker : ::std::tuple<Func, T...>
        {
            using base = ::std::tuple<Func, T...>;
            using base::base;

            template<
                typename Tuple,
                typename... Args,
                typename ForwardFunc = decltype(::std::get<0>(::std::declval<Tuple>())),
                bool Noexcept_ =
                    concepts::nothrow_invocable<ForwardFunc, T..., Args...> // clang-format on
                >
                requires ::std::invocable<ForwardFunc, T..., Args...>
            static constexpr decltype(auto) invoke_impl(Tuple&& instance, Args&&... args) //
                noexcept(Noexcept_)
            {
                return ::std::apply(
                    [&]<typename... U>(U && ... u) noexcept(Noexcept_)->decltype(auto) //
                    {
                        return ::std::invoke(
                            ::std::forward<U>(u)...,
                            ::std::forward<Args>(args)... //
                        );
                    },
                    ::std::forward<Tuple>(instance) //
                );
            }

        public:
#define BS_BIND_REF_OPERATOR(const_, ref)                                                     \
    template<typename... Args>                                                                \
        requires requires(const_ bind_ref_front_invoker::base ref instance, Args && ... args) \
        {                                                                                     \
            bind_ref_front_invoker::invoke_impl<const_ base ref>(                             \
                instance, ::std::forward<Args>(args)...);                                     \
        }                                                                                     \
    constexpr decltype(auto) operator()(Args&&... args)                                       \
        const_ ref noexcept(noexcept(bind_ref_front_invoker::invoke_impl<const_ base ref>(    \
            *this, ::std::forward<Args>(args)...)))                                           \
    {                                                                                         \
        return bind_ref_front_invoker::invoke_impl<const_ base ref>(                          \
            *this, ::std::forward<Args>(args)...);                                            \
    }

#define BS_BIND_REF_OPERATOR_PACK(const_) \
    BS_BIND_REF_OPERATOR(const_, &)       \
    BS_BIND_REF_OPERATOR(const_, &&)
            BS_BIND_REF_OPERATOR_PACK()
            BS_BIND_REF_OPERATOR_PACK(const)

#undef BS_BIND_REF_OPERATOR_PACK
#undef BS_BIND_REF_OPERATOR
        };

        template<typename Proj>
        struct projector
        {
            Proj projector;
#define BS_UTILITY_PROJECTOR_OPERATOR_DEF(const_)                                             \
    template<typename Func, typename... Args>                                                 \
        requires(::std::invocable<decltype(projected_invoke), Func, const_ Proj&, Args...>)   \
    constexpr decltype(auto) operator()(Func&& func, Args&&... args) const_ noexcept(         \
        concepts::nothrow_invocable<decltype(projected_invoke), Func, const_ Proj&, Args...>) \
    {                                                                                         \
        return projected_invoke(                                                              \
            ::std::forward<Func>(func), projector, ::std::forward<Args>(args)...);            \
    }

            BS_UTILITY_PROJECTOR_OPERATOR_DEF(const)
            BS_UTILITY_PROJECTOR_OPERATOR_DEF()

#undef BS_UTILITY_PROJECTOR_OPERATOR_DEF
        };

        template<bool Condition>
        struct conditional_invoke_fn
        {
            template<::std::invocable Func>
                requires(nodiscard_func_obj<Func>&& Condition)
            [[nodiscard]] constexpr decltype(auto) operator()(Func&& func, const auto&) const
                noexcept(concepts::nothrow_invocable<Func>)
            {
                return func();
            }

            template<::std::invocable Func>
            requires Condition constexpr decltype(auto) operator()(Func&& func, const auto&) const
                noexcept(concepts::nothrow_invocable<Func>)
            {
                return func();
            }

            template<::std::invocable Func>
                requires nodiscard_func_obj<Func>
            [[nodiscard]] constexpr decltype(auto) operator()(const auto&, Func&& func) const
                noexcept(concepts::nothrow_invocable<Func>)
            {
                return func();
            }

            template<::std::invocable Func>
            constexpr decltype(auto) operator()(const auto&, Func&& func) const
                noexcept(concepts::nothrow_invocable<Func>)
            {
                return func();
            }
        };

        struct optional_invoke_fn
        {
            template<::std::invocable Func>
            constexpr decltype(auto) operator()(Func&& func) const
                noexcept(concepts::nothrow_invocable<Func>)
            {
                return func();
            }

            template<::std::invocable Func>
                requires nodiscard_func_obj<Func>
            [[nodiscard]] constexpr decltype(auto) operator()(Func&& func) const
                noexcept(concepts::nothrow_invocable<Func>)
            {
                return func();
            }

            constexpr void operator()(const auto&) noexcept {}
        };
    }

    template<bool Condition>
    inline constexpr details::conditional_invoke_fn<Condition> conditional_invoke{};

    inline constexpr details::optional_invoke_fn optional_invoke{};

    inline constexpr struct : nodiscard_tag_t
    {
        template< //
            typename Func,
            typename... Args,
            typename Invoker = details::
                bind_ref_front_invoker<Func, type_traits::coerce_t<Args>...> // clang-format off
        > requires (::std::constructible_from<Invoker, Func, type_traits::coerce_t<Args>...> &&
            (!nodiscard_func_obj<Func> || ::std::move_constructible<Invoker>)) // clang-format on
        constexpr auto operator()(Func&& func, Args&&... args) const noexcept( //
            concepts::nothrow_constructible_from<Invoker, Func, type_traits::coerce_t<Args>...> &&
            (!nodiscard_func_obj<Func> || concepts::nothrow_move_constructible<Invoker>)
            // clang-format off
        ) // clang-format on
        {
            Invoker obj{
                ::std::forward<Func>(func),
                ::ranges::coerce<Args>{}(::std::forward<Args>(args))... //
            };

            if constexpr(nodiscard_func_obj<Func>)
                return invocable_obj{nodiscard_tag, ::std::move(obj)}; // clang-format off
            else return obj;
        }
    } bind_ref_front{};

    inline constexpr invocable_obj returnable_invoke(
        nodiscard_tag,
        []<typename Func, typename... Args> requires ::std::invocable<Func, Args...> //
        (Func&& func, Args&&... args) // clang-format off
            noexcept(concepts::nothrow_invocable<Func, Args...>) // clang-format on
            ->decltype(auto) //
        {
            const auto invoker =
                bind_ref_front(::std::forward<Func>(func), ::std::forward<Args>(args)...);
            if constexpr(::std::same_as<::std::invoke_result_t<decltype(invoker)>, void>)
            {
                invoker();
                return type_traits::empty;
            } // clang-format off
            else return invoker(); // clang-format on
        } //
    );

    template<template<typename...> typename Tuple = ::std::tuple>
    inline constexpr invocable_obj merge_invoke(
        nodiscard_tag,
        []<::std::invocable... Func>(Func&&... func) noexcept( //
            noexcept( // clang-format off
                Tuple<::std::invoke_result_t<
                    decltype(returnable_invoke),
                    Func>...
                 >{returnable_invoke(::std::forward<Func>(func))...}
            )
        ) -> Tuple<
            ::std::invoke_result_t<
                decltype(returnable_invoke),
                Func
            >...
        > // clang-format on
        {
            return {returnable_invoke(::std::forward<Func>(func))...}; //
        } //
    );

    inline constexpr invocable_obj make_returnable(
        nodiscard_tag,
        []<typename Func> // clang-format off
            requires requires(Func&& func)
            {
                ::std::bind_front(
                    returnable_invoke,
                    ::std::forward<Func>(func)
                );
            }
        (Func&& func) noexcept(
            noexcept(::std::bind_front(returnable_invoke, ::std::forward<Func>(func)))
        )
        {
            return ::std::bind_front(returnable_invoke, ::std::forward<Func>(func));
        } // clang-format on
    );

    inline constexpr invocable_obj make_projector(
        nodiscard_tag,
        []<typename Func>(Func&& func) //
        noexcept(noexcept(details::projector{::std::forward<Func>(func)}))
        {
            return details::projector{::std::forward<Func>(func)}; //
        } //
    );

    template<::std::size_t N>
    inline constexpr invocable_obj get_from_param_pack(
        nodiscard_tag,
        []<typename... Args>(Args&&... args) noexcept->decltype(auto) //
        {
            return ::std::tuple<Args&&...>(args...).get(N); //
        } //
    );
}
