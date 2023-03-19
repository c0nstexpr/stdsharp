#pragma once

#include "../type_traits/indexed_traits.h"

namespace stdsharp
{
    template<typename... T>
    using std_bind_t = decltype(::std::bind(::std::declval<T>()...));

    template<typename... T>
    concept std_bindable = requires { ::std::bind(::std::declval<T>()...); };

    template<typename... T>
    concept nothrow_std_bindable = noexcept(::std::bind(::std::declval<T>()...));

    template<typename Func, typename... T>
    class bind_t : indexed_values<Func, T...>
    {
        using base = indexed_values<Func, T...>;

    public:
        using base::base;

#define STDSHARP_OPERATOR(const_, ref)                                                    \
                                                                                          \
public:                                                                                   \
    template<typename... Args>                                                            \
        requires ::std::invocable<const_ Func ref, const_ T ref..., Args...>              \
    constexpr decltype(auto) operator()(Args&&... args)                                   \
        const_ ref noexcept(nothrow_invocable<const_ Func ref, const_ T ref..., Args...>) \
    {                                                                                     \
        return []<::std::size_t... I>(                                                    \
                   const ::std::index_sequence<I...>,                                     \
                   const_ base ref indexed,                                               \
                   Args&&... args                                                         \
        )                                                                                 \
            ->decltype(auto)                                                              \
        {                                                                                 \
            return ::std::invoke(                                                         \
                stdsharp::get<I>(static_cast<const_ base ref>(indexed))...,               \
                ::std::forward<Args>(args)...                                             \
            );                                                                            \
        }                                                                                 \
        (::std::index_sequence_for<Func, T...>{},                                         \
         static_cast<const_ base ref>(*this),                                             \
         ::std::forward<Args>(args)...);                                                  \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename Func, typename... Args>
    bind_t(Func&&, Args&&...) -> bind_t<::std::decay_t<Func>, persist_t<Args&&>...>;

    inline constexpr struct bind_fn
    {
        template<typename Func, typename... Args>
            requires requires //
        {
            bind_t{::std::declval<Func>(), ::std::declval<Args>()...};
        }
        constexpr auto operator()(Func&& func, Args&&... args) const
            noexcept(noexcept(bind_t{::std::declval<Func>(), ::std::declval<Args>()...}))
        {
            return bind_t{::std::forward<Func>(func), ::std::forward<Args>(args)...};
        }
    } bind{};

    template<typename... Args>
    concept bindable = ::std::invocable<bind_fn, Args...>;

    template<typename... Args>
    concept nothrow_bindable = nothrow_invocable<bind_fn, Args...>;
}