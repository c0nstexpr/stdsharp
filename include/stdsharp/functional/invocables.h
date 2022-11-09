//
// Created by BlurringShadow on 2021-10-15.
//
#pragma once

#include <algorithm>

#include "../type_traits/core_traits.h"

namespace stdsharp
{
    template<typename Func>
    struct invocable_t : value_wrapper<Func>
    {
        using value_wrapper<Func>::value;

#define STDSHARP_OPERATOR(const_, ref)                                                            \
    template<typename... Args>                                                                    \
        requires ::std::invocable<const_ Func ref, Args...>                                       \
    constexpr decltype(auto) operator()(Args&&... args)                                           \
        const_ ref noexcept(nothrow_invocable<const_ Func ref, Args...>)                          \
    {                                                                                             \
        return ::std::invoke(static_cast<const_ Func ref>(value), ::std::forward<Args>(args)...); \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename... Func>
    struct invocables : indexed_types<invocable_t<Func>...>
    {
        template<typename... Args>
        static constexpr ::std::array invoke_result{::std::invocable<Func, Args...>...};
    };

    namespace details
    {
#define LIKE_INVOCABLES(const_, ref)                                                         \
    template<auto Index, typename... Func>                                                   \
    consteval const_ typename invocables<Func...>::template type<Index> ref like_invocables( \
        const_ invocables<Func...> ref                                                       \
    );

        LIKE_INVOCABLES(, &)
        LIKE_INVOCABLES(const, &)
        LIKE_INVOCABLES(, &&)
        LIKE_INVOCABLES(const, &&)

#undef LIKE_INVOCABLES
    }

    template<auto Index>
    struct invoke_at_fn
    {
        template<
            typename T,
            typename... Args,
            ::std::invocable<Args...> Invocable =
                decltype(details::like_invocables<Index>(::std::declval<T>())) // clang-format off
        > // clang-format on
        constexpr decltype(auto) operator()(T&& t, Args&&... args) const
            noexcept(nothrow_invocable<Invocable, Args...>)
        {
            return get<Index>(::std::forward<T>(t))(::std ::forward<Args>(args)...);
        }
    };

    template<auto Index>
    static constexpr invoke_at_fn<Index> invoke_at{};

    template<typename... Func>
    struct sequenced_invocables : invocables<Func...>
    {
#define STDSHARP_OPERATOR(const_, ref)                                                     \
    template<                                                                              \
        typename... Args,                                                                  \
        auto InvokeResult = sequenced_invocables::template invoke_result<Args...>,         \
        typename InvokeAt =                                                                \
            invoke_at_fn<::std::ranges::find(InvokeResult, true) - InvokeResult.cbegin()>, \
        typename This = const_ sequenced_invocables ref>                                   \
        requires ::std::invocable<InvokeAt, This, Args...>                                 \
    constexpr decltype(auto) operator()(Args&&... args)                                    \
        const_ ref noexcept(nothrow_invocable<InvokeAt, This, Args...>)                    \
    {                                                                                      \
        return InvokeAt{}(static_cast<This>(*this), ::std::forward<Args>(args)...);        \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename... Func>
    sequenced_invocables(Func&&...) -> sequenced_invocables<::std::decay_t<Func>...>;

    template<typename... Func>
    struct trivial_invocables : invocables<Func...>
    {
#define STDSHARP_OPERATOR(const_, ref)                                                         \
    template<                                                                                  \
        typename... Args,                                                                      \
        auto InvokeResult = trivial_invocables::template invoke_result<Args...>,               \
        typename InvokeAt =                                                                    \
            invoke_at_fn<::std::ranges::find(InvokeResult, true) - InvokeResult.cbegin()>,     \
        typename This = const_ trivial_invocables ref>                                         \
        requires ::std::invocable<InvokeAt, This, Args...> &&                                  \
        (::std::ranges::count(trivial_invocables::template invoke_result<Args...>, true) == 1) \
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
    trivial_invocables(Func&&...) -> trivial_invocables<::std::decay_t<Func>...>;

    template<typename Func>
    struct nodiscard_invocable : invocable_t<Func>
    {
        using base = invocable_t<Func>;

#define STDSHARP_OPERATOR(const_, ref)                                             \
    template<typename... Args>                                                     \
        requires ::std::invocable<const_ base ref, Args...>                        \
    [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args)              \
        const_ ref noexcept(nothrow_invocable<const_ Func ref, Args...>)           \
    {                                                                              \
        return static_cast<const_ base ref>(*this)(::std::forward<Args>(args)...); \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename Func>
    nodiscard_invocable(Func&& func) -> nodiscard_invocable<::std::decay_t<Func>>;

    inline constexpr struct make_trivial_invocables_fn
    {
        template<typename... Invocable>
            requires requires { trivial_invocables{::std::declval<Invocable>()...}; }
        [[nodiscard]] constexpr auto operator()(Invocable&&... invocable) const
            noexcept(noexcept(trivial_invocables{::std::declval<Invocable>()...}))
        {
            return trivial_invocables{::std::forward<Invocable>(invocable)...};
        }
    } make_trivial_invocables{};

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