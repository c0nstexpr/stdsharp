#pragma once

#include <functional>

#include <range/v3/functional/overload.hpp>
#include "type_traits/type_traits.h"

#include "functional/operation.h"

namespace stdsharp::functional
{
    namespace details
    {
        template<typename Func, typename... T>
        class [[nodiscard]] bind_ref_front_invoker : ::std::tuple<Func, T...>
        {
            using base = ::std::tuple<Func, T...>;
            using base::base;

            template<typename... Args>
            static constexpr bool noexcept_v =
                ::stdsharp::concepts::nothrow_invocable<Func, T..., Args...>;

        public:
#define BS_BIND_REF_OPERATOR(const_, ref)                                                        \
    template<typename... Args>                                                                   \
        requires ::std::invocable<const Func, const T..., Args...>                               \
    constexpr decltype(auto) operator()(Args&&... args)                                          \
        const_ ref noexcept(bind_ref_front_invoker::noexcept_v<Args...>)                         \
    {                                                                                            \
        return ::std::apply(                                                                     \
            [&]<typename... U>(U && ... t) noexcept(bind_ref_front_invoker::noexcept_v<Args...>) \
                ->decltype(auto)                                                                 \
            { return ::std::invoke(::std::forward<U>(t)..., ::std::forward<Args>(args)...); },   \
            static_cast<const_ base ref>(*this));                                                \
    }

#define BS_BIND_REF_OPERATOR_PACK(const_) \
    BS_BIND_REF_OPERATOR(const_, &)       \
    BS_BIND_REF_OPERATOR(const_, &&)

            BS_BIND_REF_OPERATOR_PACK()
            BS_BIND_REF_OPERATOR_PACK(const)

#undef BS_BIND_REF_OPERATOR_PACK
#undef BS_BIND_REF_OPERATOR
        };

        template<typename Func, typename... T>
        bind_ref_front_invoker(Func&&, T&&...) -> bind_ref_front_invoker<
            ::stdsharp::type_traits::coerce_t<Func>,
            ::stdsharp::type_traits::coerce_t<T>... // clang-format off
        >; // clang-format on
    }

    inline constexpr auto empty_invoke = [](auto&&...) noexcept
    {
        return ::stdsharp::type_traits::empty; //
    };

    template<typename ReturnT>
    inline constexpr ::stdsharp::functional::invocable_obj invoke_r(
        ::stdsharp::functional::nodiscard_tag,
        []<typename Func, typename... Args>(Func&& func, Args&&... args) // clang-format off
            noexcept(::stdsharp::concepts::nothrow_invocable_r<ReturnT, Func, Args...>)
            -> ::std::enable_if_t<::stdsharp::concepts::invocable_r<ReturnT, Func, Args...>, ReturnT>
        {
            return static_cast<ReturnT>(
                ::std::invoke(::std::forward<Func>(func), ::std::forward<Args>(args)...)
            );
        } // clang-format on
    );

    inline constexpr auto bind_ref_front = []<typename Func, typename... Args>(
        Func && func,
        Args&&... args // clang-format off
    ) noexcept( // clang-format on
        noexcept( //
            ::stdsharp::functional::details::bind_ref_front_invoker{
                ::std::forward<Func>(func),
                ::std::forward<Args>(args)... //
            } // clang-format off
        ) // clang-format on
    )
    {
        return ::stdsharp::functional::details::bind_ref_front_invoker{
            ::std::forward<Func>(func),
            ::std::forward<Args>(args)... //
        }; //
    };

    inline constexpr ::stdsharp::functional::invocable_obj returnable_invoke(
        nodiscard_tag,
        []<typename Func, typename... Args>(Func&& func, Args&&... args) // clang-format off
            noexcept(::stdsharp::concepts::nothrow_invocable<Func, Args...>) // clang-format on
            ->decltype(auto)
        {
            const auto invoker = ::stdsharp::functional::bind_ref_front(
                ::std::forward<Func>(func),
                ::std::forward<Args>(args)... //
            );
            if constexpr(::std::same_as<::std::invoke_result_t<decltype(invoker)>, void>)
            {
                invoker();
                return ::stdsharp::type_traits::empty;
            } // clang-format off
            else return invoker(); // clang-format on
        } //
    );

    template<template<typename...> typename Tuple = ::std::tuple>
    inline constexpr ::stdsharp::functional::invocable_obj merge_invoke(
        ::stdsharp::functional::nodiscard_tag,
        []<::std::invocable... Func>(Func&&... func) noexcept( //
            noexcept( // clang-format off
                Tuple<::std::invoke_result_t<
                    decltype(::stdsharp::functional::returnable_invoke),
                    Func>...
                 >{::stdsharp::functional::returnable_invoke(::std::forward<Func>(func))...}
            )
        ) // clang-format on
        {
            return Tuple< // clang-format off
                ::std::invoke_result_t<decltype(::stdsharp::functional::returnable_invoke), Func>...
            >{
                ::stdsharp::functional::returnable_invoke(::std::forward<Func>(func))... // clang-format on
            };
        } //
    );

    inline constexpr ::stdsharp::functional::invocable_obj make_returnable(
        ::stdsharp::functional::nodiscard_tag,
        []<typename Func>(Func&& func) noexcept( //
            noexcept( //
                ::std::bind_front(
                    ::stdsharp::functional::returnable_invoke,
                    ::std::forward<Func>(func) // clang-format off
                )
            ) // clang-format on
        )
        {
            return ::std::bind_front(
                ::stdsharp::functional::returnable_invoke,
                ::std::forward<Func>(func) //
            );
        } //
    );

    template<typename, typename...>
    struct cpo_invoke_fn;

    namespace details
    {
        template<typename Proj>
        class [[nodiscard]] projector : invocable_obj<Proj>
        {
            using base = invocable_obj<Proj>;

        public:
            using base::base;

#define BS_UTILITY_PROJECTOR_OPERATOR_DEF(const_)                                              \
    template<typename Func, typename... Args>                                                  \
        requires(::std::invocable<const_ base, Args> && ...)                                   \
    &&::std::                                                                                  \
        invocable<Func, ::std::invoke_result_t<const_ base, Args>...> constexpr decltype(auto) \
            operator()(Func&& func, Args&&... args) const_ noexcept(                           \
                ::stdsharp::concepts::                                                         \
                    nothrow_invocable<Func, ::std::invoke_result_t<const base, Args>...>)      \
    {                                                                                          \
        return ::std::invoke(                                                                  \
            ::std::forward<Func>(func), base::operator()(::std::forward<Args>(args))...);      \
    }

            BS_UTILITY_PROJECTOR_OPERATOR_DEF(const)
            BS_UTILITY_PROJECTOR_OPERATOR_DEF()
#undef BS_UTILITY_PROJECTOR_OPERATOR_DEF
        };

        template<typename CPOTag>
        struct cpo_fn
        {
            template<typename... T>
            constexpr decltype(auto) operator()(T&&... t) const noexcept
            {
                return ::stdsharp::functional::cpo_invoke_fn<CPOTag, ::std::remove_const_t<T>...>::
                    invoke(::std::forward<T>(t)... // clang-format off
                ); // clang-format on
            }
        };
    }

    inline constexpr auto make_projector = []<typename Func>(Func&& func)
    {
        return ::stdsharp::functional::details::projector<Func>{::std::forward<Func>(func)}; //
    };

    template<typename CPOTag>
    inline constexpr ::stdsharp::functional::details::cpo_fn<CPOTag> cpo{};
}
