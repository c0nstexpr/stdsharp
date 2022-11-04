//
// Created by BlurringShadow on 2021-10-15.
//
#pragma once

#include <algorithm>

#include "../type_traits/core_traits.h"

namespace stdsharp::functional
{
    template<typename Func>
    class invocable : value_wrapper<Func>
    {
        using base = value_wrapper<Func>;

    public:
        using base::base;

#define STDSHARP_OPERATOR(const_, ref)                                             \
    template<typename... Args>                                                     \
        requires ::std::invocable<const_ Func ref, Args...>                        \
    constexpr decltype(auto) operator()(Args&&... args)                            \
        const_ ref noexcept(concepts::nothrow_invocable<const_ Func ref, Args...>) \
    {                                                                              \
        return ::std::invoke(                                                      \
            static_cast<const_ Func ref>(base::value),                             \
            ::std::forward<Args>(args)...                                          \
        );                                                                         \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<auto Index>
    struct invoke_at_fn
    {
#define STDSHARP_OPERATOR(const_, ref)                                                    \
    template<                                                                             \
        typename... Args,                                                                 \
        typename... Func,                                                                 \
        typename Invocables = type_traits::indexed_types<invocable<Func>...>,             \
        typename Invocable = const_ typename Invocables::template type<Index> ref>        \
        requires ::std::invocable<Invocable, Args...>                                     \
    constexpr decltype(auto) operator()(const_ Invocables ref invocables, Args&&... args) \
        const noexcept(concepts::nothrow_invocable<Invocable, Args...>)                   \
    {                                                                                     \
        return get<Index>(static_cast<const_ Invocables ref>(invocables)                  \
        )(::std::forward<Args>(args)...);                                                 \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<auto Index>
    static constexpr invoke_at_fn<Index> invoke_at{};

    template<typename... Func>
    struct invocables : type_traits::indexed_types<invocable<Func>...>
    {
        template<typename... Args>
        static constexpr ::std::array invoke_result{::std::invocable<Func, Args...>...};
    };

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
        const_ ref noexcept(concepts::nothrow_invocable<InvokeAt, This, Args...>)          \
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
    class trivial_invocables : sequenced_invocables<Func...>
    {
        using base = sequenced_invocables<Func...>;

    public:
        trivial_invocables() = default;

        template<typename... F>
            requires concepts::list_initializable_from<base, F...>
        constexpr trivial_invocables(F&&... f) //
            noexcept(concepts::nothrow_list_initializable_from<base, F...>):
            base{::std::forward<F>(f)...}
        {
        }

        template<typename... Args>
        static constexpr auto invoke_result = base::template invoke_result<Args...>;

#define STDSHARP_OPERATOR(const_, ref)                                  \
    template<typename... Args, typename This = const_ base ref>         \
        requires ::std::invocable<This, Args...> &&                     \
        (::std::ranges::count(invoke_result<Args...>, true) == 1)       \
    constexpr decltype(auto) operator()(Args&&... args)                 \
        const_ ref noexcept(concepts::nothrow_invocable<This, Args...>) \
    {                                                                   \
        return static_cast<This>(*this)(::std::forward<Args>(args)...); \
    }                                                                   \
                                                                        \
    constexpr operator const_ invocables<Func...> ref() const_ ref      \
    {                                                                   \
        return static_cast<const_ invocables<Func...> ref>(*this);      \
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
    class nodiscard_invocable : invocable<Func>
    {
        using base = invocable<Func>;

    public:
        using base::base;

#define STDSHARP_OPERATOR(const_, ref)                                             \
    template<typename... Args>                                                     \
        requires ::std::invocable<const_ base ref, Args...>                        \
    [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args)              \
        const_ ref noexcept(concepts::nothrow_invocable<const_ Func ref, Args...>) \
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