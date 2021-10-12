#pragma once

#include <functional>

#include <range/v3/functional.hpp>

#include "type_traits/type_traits.h"
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

            template<typename CFunc, typename... Args>
                requires ::std::invocable<CFunc, T..., Args...>
            static constexpr bool noexcept_v_ =
                ::stdsharp::concepts::nothrow_invocable<CFunc, T..., Args...>;

            template<
                typename Tuple,
                typename... Args,
                bool Noexcept_ = bind_ref_front_invoker::noexcept_v_<
                    ::std::conditional_t<::stdsharp::concepts::const_<Tuple>, const Func, Func>,
                    Args... // clang-format off
                >
            > // clang-format on
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
#define BS_BIND_REF_OPERATOR(const_, ref)                                                  \
    template<typename... Args>                                                             \
        requires requires(const_ base ref instance, Args && ... args)                      \
        {                                                                                  \
            bind_ref_front_invoker::invoke_impl<const_ base ref>(                          \
                instance, ::std::forward<Args>(args)...);                                  \
        }                                                                                  \
    constexpr decltype(auto) operator()(Args&&... args)                                    \
        const_ ref noexcept(noexcept(bind_ref_front_invoker::invoke_impl<const_ base ref>( \
            *this, ::std::forward<Args>(args)...)))                                        \
    {                                                                                      \
        return bind_ref_front_invoker::invoke_impl<const_ base ref>(                       \
            *this, ::std::forward<Args>(args)...);                                         \
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
            Func,
            ::stdsharp::type_traits::coerce_t<T>... // clang-format off
        >; // clang-format on

        template<typename Proj>
        class projector : invocable_obj<Proj>
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
    }

    inline constexpr auto empty_invoke = [](auto&&...) noexcept
    {
        return ::stdsharp::type_traits::empty; //
    };

    template<typename ReturnT>
    inline constexpr ::stdsharp::functional::invocable_obj invoke_r(
        ::stdsharp::functional::nodiscard_tag,
        []<typename Func, typename... Args>(Func&& func, Args&&... args) // clang-format off
            noexcept(::stdsharp::concepts::nothrow_invocable_r<Func, ReturnT, Args...>)
            -> ::std::enable_if_t<::stdsharp::concepts::invocable_r<Func, ReturnT, Args...>, ReturnT>
        {
            return static_cast<ReturnT>(
                ::std::invoke(::std::forward<Func>(func), ::std::forward<Args>(args)...)
            );
        } // clang-format on
    );

    inline constexpr ::stdsharp::functional::invocable_obj bind_ref_front(
        ::stdsharp::functional::nodiscard_tag,
        []<typename Func, typename... Args> requires requires //
        {
            ::stdsharp::functional::details:: //
                bind_ref_front_invoker{::std::declval<Func>(), ::std::declval<Args>()...};
        } // clang-format off
        (Func&& func, Args&&... args) noexcept(noexcept(::stdsharp::functional::details::
        bind_ref_front_invoker{::std::forward<Func>(func),::std::forward<Args>(args)...})) // clang-format on
        {
            auto&& obj = ::stdsharp::functional::details:: //
                bind_ref_front_invoker{::std::forward<Func>(func), ::std::forward<Args>(args)...};

            if constexpr(::stdsharp::functional::nodiscard_func_obj<Func>)
                return ::stdsharp::functional::invocable_obj{
                    ::stdsharp::functional::nodiscard_tag,
                    ::std::move(obj) //
                }; // clang-format off
            else return obj;
        } // clang-format on
    );

    inline constexpr ::stdsharp::functional::invocable_obj returnable_invoke(
        nodiscard_tag,
        []<typename Func, typename... Args> requires ::std::invocable<Func, Args...> //
        (Func&& func, Args&&... args) // clang-format off
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
        ) -> Tuple<
            ::std::invoke_result_t<
                decltype(::stdsharp::functional::returnable_invoke),
                Func
            >...
        > // clang-format on
        {
            return {::stdsharp::functional::returnable_invoke(::std::forward<Func>(func))...}; //
        } //
    );

    inline constexpr ::stdsharp::functional::invocable_obj make_returnable(
        ::stdsharp::functional::nodiscard_tag,
        []<typename Func> // clang-format off
            requires requires(Func&& func)
            {
                ::std::bind_front(
                    ::stdsharp::functional::returnable_invoke,
                    ::std::forward<Func>(func)
                );
            } // clang-format on
        (Func&& func) noexcept( //
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

    inline constexpr ::stdsharp::functional::invocable_obj make_projector(
        ::stdsharp::functional::nodiscard_tag,
        []<typename Func>(Func&& func)
        {
            return ::stdsharp::functional::details::projector<Func>{::std::forward<Func>(func)}; //
        } //
    );

    namespace details
    {
        template<typename T, auto Count = T::size>
        struct split_into_fn : T
        {
            using T::T;
            using T::operator();

            explicit constexpr split_into_fn(const T& t) //
                noexcept(::stdsharp::concepts::nothrow_copy_constructible<T>):
                T(t)
            {
            }

            explicit constexpr split_into_fn(T&& t) //
                noexcept(::stdsharp::concepts::nothrow_move_constructible<T>):
                T(::std::move(t))
            {
            }

#define BS_SPLIT_INTO_OPERATOR(const_)                                                         \
    template<typename Fn>                                                                      \
    [[nodiscard]] constexpr auto operator()(Fn&& fn) const_ noexcept(                          \
        noexcept((*this)(::std::forward<Fn>(fn), ::std::make_index_sequence<Count>{})))        \
    {                                                                                          \
        return (*this)(::std::forward<Fn>(fn), ::std::make_index_sequence<Count>{});           \
    }                                                                                          \
                                                                                               \
private:                                                                                       \
    template<typename Fn, ::std::size_t... I>                                                  \
    constexpr auto operator()(Fn&& fn, const ::std::index_sequence<I...>)                      \
        const_ noexcept(noexcept(::stdsharp::functional::bind_ref_front(                       \
            ::std::declval<Fn>(), (*this)(::stdsharp::type_traits::index_constant<I>{})...)))  \
    {                                                                                          \
        return ::stdsharp::functional::bind_ref_front(                                         \
            ::std::forward<Fn>(fn), (*this)(::stdsharp::type_traits::index_constant<I>{})...); \
    }                                                                                          \
                                                                                               \
public:

            BS_SPLIT_INTO_OPERATOR()
            BS_SPLIT_INTO_OPERATOR(const)

#undef BS_SPLIT_INTO_OPERATOR

        private:
            template<::stdsharp::concepts::decay_same_as<split_into_fn> This, typename Fn>
            friend auto operator|(This&& instance, Fn&& fn) //
                noexcept(::stdsharp::concepts::nothrow_invocable<This, Fn>)
            {
                return ::std::forward<This>(instance)(::std::forward<Fn>(fn));
            }
        };

        template<typename T>
        split_into_fn(T&& t) -> split_into_fn<::std::decay_t<T>>;

        inline constexpr struct
        {
            template<typename T, typename U, typename ResT = ::std::invoke_result_t<T, U>>
                requires requires(ResT&& res)
                {
                    ::stdsharp::functional::details::split_into_fn{::std::move(res)};
                }
            constexpr auto operator()(T&& t, U&& u) const noexcept( //
                noexcept( //
                    ::stdsharp::functional::details:: // clang-format off
                        split_into_fn{::std::invoke(::std::forward<T>(t), ::std::forward<U>(u))}
                ) // clang-format on
            )
            {
                return ::stdsharp::functional::details:: //
                    split_into_fn{::std::invoke(::std::forward<T>(t), ::std::forward<U>(u))};
            }

        } make_split_into{};
    }

    inline constexpr ::stdsharp::functional::invocable_obj split_into(
        ::stdsharp::functional::nodiscard_tag,
        []<typename T>(T&& t)
        {
            return ::ranges::make_pipeable( //
                ::std::bind_front(
                    ::stdsharp::functional::details::make_split_into,
                    ::std::forward<T>(t) // clang-format off
                ) // clang-format on
            );
        } //
    );
}
