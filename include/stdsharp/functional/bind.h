#pragma once

#include "../tuple/tuple.h"

namespace stdsharp
{
    template<typename... T>
    using std_bind_t = decltype(::std::bind(::std::declval<T>()...));

    template<typename... T>
    concept std_bindable = requires { ::std::bind(::std::declval<T>()...); };

    template<typename... T>
    concept nothrow_std_bindable = noexcept(::std::bind(::std::declval<T>()...));

    template<typename Func, typename... T>
    class bind_t : ::std::tuple<Func, T...>
    {
        using base = ::std::tuple<Func, T...>;

    public:
        using base::base;

#define STDSHARP_OPERATOR(const_, ref)                                                     \
                                                                                           \
private:                                                                                   \
    template<::std::size_t... I, typename... Args>                                         \
    constexpr decltype(auto) operator()(const ::std::index_sequence<I...>, Args&&... args) \
        const_ ref noexcept(nothrow_invocable<const_ Func ref, const_ T ref..., Args...>)  \
    {                                                                                      \
        decltype(auto) tuple = static_cast<const_ base ref>(*this);                        \
        return ::std::invoke(::std::get<I>(tuple)..., ::std::forward<Args>(args)...);      \
    }                                                                                      \
                                                                                           \
public:                                                                                    \
    template<typename... Args>                                                             \
        requires ::std::invocable<const_ Func ref, const_ T ref..., Args...>               \
    constexpr decltype(auto) operator()(Args&&... args)                                    \
        const_ ref noexcept(nothrow_invocable<const_ Func ref, const_ T ref..., Args...>)  \
    {                                                                                      \
        return static_cast<const_ bind_t ref>(*this                                        \
        )(::std::index_sequence_for<Func, T...>{}, ::std::forward<Args>(args)...);         \
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