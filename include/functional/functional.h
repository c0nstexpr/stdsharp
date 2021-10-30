#pragma once
#include <range/v3/functional.hpp>

#include "functional/operation.h"

namespace stdsharp::functional
{
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
                typename AlignedFunc = type_traits::const_ref_align_t<Tuple, Func>,
                bool Noexcept_ =
                    concepts::nothrow_invocable<AlignedFunc, T..., Args...> // clang-format on
                >
                requires ::std::invocable<AlignedFunc, T..., Args...>
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

        template<typename Func, typename... T>
        bind_ref_front_invoker(Func&&, T&&...)
            -> bind_ref_front_invoker<Func, type_traits::coerce_t<T>...>;

        template<typename Proj>
        class projector : invocable_obj<Proj>
        {
            using base = invocable_obj<Proj>;

        public:
            using base::base;

#define BS_UTILITY_PROJECTOR_OPERATOR_DEF(const_)                                               \
    template<typename Func, typename... Args>                                                   \
        requires(::std::invocable<const_ base, Args> && ...)                                    \
    &&::std::                                                                                   \
        invocable<Func, ::std::invoke_result_t<const_ base, Args>...> constexpr decltype(auto)  \
            operator()(Func&& func, Args&&... args) const_ noexcept(                            \
                concepts::nothrow_invocable<Func, ::std::invoke_result_t<const base, Args>...>) \
    {                                                                                           \
        return ::std::invoke(                                                                   \
            ::std::forward<Func>(func), base::operator()(::std::forward<Args>(args))...);       \
    }

            BS_UTILITY_PROJECTOR_OPERATOR_DEF(const)
            BS_UTILITY_PROJECTOR_OPERATOR_DEF()
#undef BS_UTILITY_PROJECTOR_OPERATOR_DEF
        };
    }

    inline constexpr auto empty_invoke = [](const auto&...) noexcept
    {
        return type_traits::empty; //
    };

    namespace details
    {
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
    }

    template<bool Condition>
    inline constexpr details::conditional_invoke_fn<Condition> conditional_invoke{};

    namespace details
    {
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

    inline constexpr details::optional_invoke_fn optional_invoke{};

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

    inline constexpr invocable_obj bind_ref_front(
        nodiscard_tag,
        []<typename Func, typename... Args> requires requires //
        {
            details::bind_ref_front_invoker{::std::declval<Func>(), ::std::declval<Args>()...};
        } // clang-format off
        (Func&& func, Args&&... args) noexcept(
            noexcept(
                details::
                    bind_ref_front_invoker{::std::forward<Func>(func),::std::forward<Args>(args)...}
            )
        )
        // clang-format on
        {
            auto&& obj = details:: //
                bind_ref_front_invoker{::std::forward<Func>(func), ::std::forward<Args>(args)...};

            if constexpr(nodiscard_func_obj<Func>)
                return invocable_obj{nodiscard_tag, ::std::move(obj)}; // clang-format off
            else return obj;
        } // clang-format on
    );

    inline constexpr invocable_obj returnable_invoke(
        nodiscard_tag,
        []<typename Func, typename... Args> requires ::std::invocable<Func, Args...> //
        (Func&& func, Args&&... args) // clang-format off
            noexcept(concepts::nothrow_invocable<Func, Args...>) // clang-format on
            ->decltype(auto)
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
        []<typename Func>(Func&& func)
        {
            return details::projector<Func>{::std::forward<Func>(func)}; //
        } //
    );
}
