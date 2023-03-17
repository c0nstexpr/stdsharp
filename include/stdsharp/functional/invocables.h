//

#pragma once

#include <algorithm>

#include "../type_traits/indexed_traits.h"
#include "../utility/invocable.h"

namespace stdsharp
{
    namespace details
    {
        template<typename T, ::std::size_t I>
        struct indexed_invocable : stdsharp::indexed_value<T, I>
        {
            using base = stdsharp::indexed_value<T, I>;

            using base::base;

#define STDSHARP_OPERATOR(const_, ref)                                \
    template<typename... Args, typename Fn = const_ T ref>            \
        requires ::std::invocable<Fn, Args...>                        \
    constexpr decltype(auto) operator()(Args&&... args)               \
        const_ ref noexcept(nothrow_invocable<Fn, Args...>)           \
    {                                                                 \
        return static_cast<Fn>(*this)(::std::forward<Args>(args)...); \
    }

            STDSHARP_OPERATOR(, &)
            STDSHARP_OPERATOR(const, &)
            STDSHARP_OPERATOR(, &&)
            STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
        };

        template<typename... Func>
        struct invocables
        {
            template<typename = ::std::index_sequence_for<Func...>>
            struct impl;

            using type = impl<>;

            template<::std::size_t... I>
            struct impl<::std::index_sequence<I...>> : indexed_invocable<Func, I>...
            {
                template<typename... Args>
                    requires(::std::constructible_from<Func, Args> && ...)
                constexpr impl(Args&&... args) //
                    noexcept((nothrow_constructible_from<Func, Args> && ...)):
                    indexed_invocable<Func, I>(::std::forward<Args>(args))...
                {
                }

                template<typename... Args>
                static constexpr ::std::array invoke_result{( //
                    ::std::invocable<Func, Args...> ? //
                        nothrow_invocable<Func, Args...> ? //
                            expr_req::no_exception :
                            expr_req::well_formed :
                        expr_req::ill_formed
                )...};
            };
        };

        template<typename... Func>
        using invocables_t = ::meta::_t<invocables<Func...>>;
    }

    template<typename... Func>
    struct invocables : details::invocables_t<Func...>
    {
        using details::invocables_t<Func...>::invocables_t;
    };

    template<typename... Func>
    invocables(Func&&...) -> invocables<::std::decay_t<Func>...>;

    inline constexpr struct make_invocables_fn
    {
        template<typename... Invocable>
            requires requires { invocables{::std::declval<Invocable>()...}; }
        [[nodiscard]] constexpr auto operator()(Invocable&&... invocable) const
            noexcept(noexcept(invocables{::std::declval<Invocable>()...}))
        {
            return invocables{::std::forward<Invocable>(invocable)...};
        }
    } make_invocables{};

    template<auto Index>
    struct invoke_at_fn
    {
        template<
            typename T,
            typename... Args,
            ::std::invocable<Args...> Invocable = get_element_t<Index, T> // clang-format off
        > // clang-format on
        constexpr decltype(auto) operator()(T&& t, Args&&... args) const
            noexcept(nothrow_invocable<Invocable, Args...>)
        {
            return cpo::get_element<Index>(::std::forward<T>(t))(::std ::forward<Args>(args)...);
        }
    };

    template<auto Index>
    static constexpr invoke_at_fn<Index> invoke_at{};

    template<typename... Func>
    struct sequenced_invocables : invocables<Func...>
    {
        using invocables<Func...>::invocables;

#define STDSHARP_OPERATOR(const_, ref)                                                         \
    template<                                                                                  \
        typename... Args,                                                                      \
        auto InvokeResult = sequenced_invocables::template invoke_result<Args...>,             \
        typename InvokeAt = invoke_at_fn<                                                      \
            ::std::ranges::find(InvokeResult, expr_req::well_formed) - InvokeResult.cbegin()>, \
        typename This = const_ sequenced_invocables ref>                                       \
        requires ::std::invocable<InvokeAt, This, Args...>                                     \
    constexpr decltype(auto) operator()(Args&&... args)                                        \
        const_ ref noexcept(nothrow_invocable<InvokeAt, This, Args...>)                        \
    {                                                                                          \
        return InvokeAt{}(static_cast<This>(*this), ::std::forward<Args>(args)...);            \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename... Func>
    sequenced_invocables(Func&&...) -> sequenced_invocables<::std::decay_t<Func>...>;

    inline constexpr struct make_sequenced_invocables_fn
    {
        template<typename... Invocable>
            requires requires { sequenced_invocables{::std::declval<Invocable>()...}; }
        [[nodiscard]] constexpr auto operator()(Invocable&&... invocable) const
            noexcept(noexcept(sequenced_invocables{::std::declval<Invocable>()...}))
        {
            return sequenced_invocables{::std::forward<Invocable>(invocable)...};
        }
    } make_sequenced_invocables{};
}