//

#pragma once

#include <algorithm>
#include <functional>

#include "../type_traits/indexed_traits.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename... Func>
    struct invocables_traits
    {
        using indexed_values = stdsharp::indexed_values<Func...>;

        template<typename T, std::size_t I>
        struct indexed_operator
        {
        private:
            using func = std::tuple_element_t<I, indexed_values>;

            template<typename This, typename... Args, typename Fn = cv_ref_align_t<This&&, func>>
                requires ::std::invocable<Fn, Args...>
            static constexpr decltype(auto) operator_impl(This&& this_, Args&&... args) //
                noexcept(nothrow_invocable<Fn, Args...>)
            {
                return std::invoke(
                    cpo::get_element<I>( //
                        static_cast<cv_ref_align_t<This&&, indexed_values>>(
                            static_cast<cv_ref_align_t<This&&, type>>(this_)
                        )
                    ),
                    cpp_forward(args)...
                );
            }

        public:
            STDSHARP_MEM_PACK(operator(), operator_impl, indexed_operator)
        };

        template<typename = std::index_sequence_for<Func...>>
        struct impl;

        using type = impl<>;

        template<std::size_t... I>
        struct impl<std::index_sequence<I...>> :
            indexed_operator<indexed_values, I>...,
            stdsharp::indexed_values<Func...>
        {
            using stdsharp::indexed_values<Func...>::indexed_values;
            using indexed_operator<indexed_values, I>::operator()...;
        };
    };

    template<typename... Func>
    using invocables = invocables_traits<Func...>::type;
}

namespace stdsharp
{
    template<typename... Func>
    struct invocables : details::invocables<Func...>
    {
        using details::invocables<Func...>::invocables;
    };

    template<typename... T>
    invocables(T&&...) -> invocables<std::decay_t<T>...>;

    template<std::size_t Index>
    struct invoke_at_fn
    {
        template<typename T, typename... Args>
            requires std::invocable<get_element_t<Index, T>, Args...>
        constexpr decltype(auto) operator()(T && t, Args&&... args) const
            noexcept(nothrow_invocable<get_element_t<Index, T>, Args...>)
        {
            return cpo::get_element<Index>(t)(cpp_forward(args)...);
        }
    };

    template<auto Index>
    inline constexpr invoke_at_fn<Index> invoke_at{};

    template<template<typename> typename Predicator>
    struct invoke_first_fn
    {
    private:
        template<typename T, typename... Args, std::size_t I = 0>
        static constexpr std::size_t find_first(const index_constant<I> = {}) noexcept
        {
            if constexpr( //
                requires {
                    requires I < std::tuple_size_v<std::decay_t<T>>;
                    typename get_element_t<I, T>;
                } //
            )
                if constexpr(Predicator<get_element_t<I, T>>::template value<Args...>) return I;
                else return find_first<T, Args...>(index_constant<I + 1>{});
            else return static_cast<std::size_t>(-1);
        }

        template<typename T, typename... Args>
        static constexpr auto index = find_first<T, Args...>();

        template<typename T, typename... Args>
            requires(index<T, Args...> != -1)
        using invoke_at_t = invoke_at_fn<index<T, Args...>>;

    public:
        template<typename T, typename... Args>
            requires requires { typename invoke_at_t<T, Args...>; }
        constexpr decltype(auto) operator()(T && t, Args&&... args) const
            noexcept(nothrow_invocable<invoke_at_t<T, Args...>, T, Args...>)
        {
            return invoke_at_t<T, Args...>{}(t, cpp_forward(args)...);
        }
    };

    template<template<typename> typename Predicator>
    inline constexpr invoke_first_fn<Predicator> invoke_first{};

    namespace details
    {
        template<typename Func>
        struct sequenced_invocables_predicate
        {
            template<typename... Args>
            static constexpr auto value = std::invocable<Func, Args...>;
        };
    }

    using sequenced_invoke_fn = invoke_first_fn<details::sequenced_invocables_predicate>;

    inline constexpr sequenced_invoke_fn sequenced_invoke{};

    template<typename... Invocable>
    struct sequenced_invocables : invocables<Invocable...>
    {
        using base = invocables<Invocable...>;

        using base::base;

    private:
        template<typename This, typename... Args, typename Base = cv_ref_align_t<This&&, base>>
            requires std::invocable<sequenced_invoke_fn, Base, Args...>
        static constexpr decltype(auto) operator_impl(This&& this_, Args&&... args) //
            noexcept(nothrow_invocable<sequenced_invoke_fn, Base, Args...>)
        {
            return sequenced_invoke(static_cast<Base>(this_), cpp_forward(args)...);
        }

    public:
        STDSHARP_MEM_PACK(operator(), operator_impl, sequenced_invocables)
    };

    template<typename... T>
    sequenced_invocables(T&&...) -> sequenced_invocables<std::decay_t<T>...>;

    template<typename Func>
    struct nodiscard_invocable : invocables<Func>
    {
        using base = invocables<Func>;

        using base::base;

#define STDSHARP_NODISCARD_OPERATOR(cv, ref)                          \
    template<typename... Args, typename Base = cv base ref>           \
        requires std::invocable<Base, Args...>                        \
    [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) \
        cv ref noexcept(nothrow_invocable<cv Func ref, Args...>)      \
    {                                                                 \
        return static_cast<Base>(*this)(cpp_forward(args)...);        \
    }

        STDSHARP_NODISCARD_OPERATOR(, &)
        STDSHARP_NODISCARD_OPERATOR(const, &)
        STDSHARP_NODISCARD_OPERATOR(, &&)
        STDSHARP_NODISCARD_OPERATOR(const, &&)
        STDSHARP_NODISCARD_OPERATOR(volatile, &)
        STDSHARP_NODISCARD_OPERATOR(const volatile, &)
        STDSHARP_NODISCARD_OPERATOR(volatile, &&)
        STDSHARP_NODISCARD_OPERATOR(const volatile, &&)

#undef STDSHARP_NODISCARD_OPERATOR
    };

    template<typename Func>
    nodiscard_invocable(Func&&) -> nodiscard_invocable<std::decay_t<Func>>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::invocables<T...>> :
        ::std::tuple_size<::stdsharp::indexed_types<T...>>
    {
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::invocables<T...>> :
        ::std::tuple_element<I, ::stdsharp::indexed_types<T...>>
    {
    };

    template<typename... T>
    struct tuple_size<::stdsharp::sequenced_invocables<T...>> :
        ::std::tuple_size<::stdsharp::invocables<T...>>
    {
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::sequenced_invocables<T...>> :
        ::std::tuple_element<I, ::stdsharp::invocables<T...>>
    {
    };
}

#include "../compilation_config_out.h"