#pragma once

#include "../tuple/tuple.h"

namespace stdsharp::functional
{
    template<typename... T>
    using std_bind_t = decltype(::std::bind(::std::declval<T>()...));

    template<typename... T>
    concept std_bindable = requires
    {
        ::std::bind(::std::declval<T>()...);
    };

    template<typename... T>
    concept nothrow_std_bindable = noexcept(::std::bind(::std::declval<T>()...));

    template<typename Func, typename... T>
    class bind_t : ::std::tuple<Func, T...>
    {
        using base = ::std::tuple<Func, T...>;

    public:
        template<typename... U>
            requires ::std::constructible_from<base, U...>
        constexpr bind_t(U&&... u) noexcept(concepts::nothrow_constructible_from<base, U...>):
            base(::std::forward<U>(u)...)
        {
        }

#define BS_OPERATOR(const_, ref)                                                           \
private:                                                                                   \
    template<::std::size_t... N, typename... Args>                                         \
    constexpr decltype(auto) operator()(const ::std::index_sequence<N...>, Args&&... args) \
        const_ ref noexcept(                                                               \
            concepts::nothrow_invocable<const_ Func ref, const_ T ref..., Args...>)        \
    {                                                                                      \
        return ::std::invoke(                                                              \
            static_cast<get_t<N, const_ base ref>>(                                        \
                ::std::get<N>(static_cast<const_ base ref>(*this)))...,                    \
            ::std::forward<Args>(args)...);                                                \
    }                                                                                      \
                                                                                           \
public:                                                                                    \
    template<typename... Args>                                                             \
        requires ::std::invocable<const_ Func ref, const_ T ref..., Args...>               \
    constexpr decltype(auto) operator()(Args&&... args) const_ ref noexcept(               \
        concepts::nothrow_invocable<const_ Func ref, const_ T ref..., Args...>)            \
    {                                                                                      \
        return static_cast<const_ bind_t ref>(*this)(                                      \
            ::std::index_sequence_for<Func, T...>{}, ::std::forward<Args>(args)...);       \
    }

        BS_OPERATOR(, &)
        BS_OPERATOR(const, &)
        BS_OPERATOR(, &&)
        BS_OPERATOR(const, &&)

#undef BS_OPERATOR
    };

    template<typename Func, typename... Args>
    bind_t(Func&&, Args&&...) -> bind_t<::std::decay_t<Func>, type_traits::persist_t<Args&&>...>;

    inline constexpr struct bind_fn
    {
        template<typename Func, typename... Args>
            requires requires { bind_t{::std::declval<Func>(), ::std::declval<Args>()...}; }
        constexpr auto operator()(Func&& func, Args&&... args) const
            noexcept(noexcept(bind_t{::std::declval<Func>(), ::std::declval<Args>()...}))
        {
            return bind_t{::std::forward<Func>(func), ::std::forward<Args>(args)...};
        }
    } bind{};

    template<typename... Args>
    concept bindable = ::std::invocable<bind_fn, Args...>;

    template<typename... Args>
    concept nothrow_bindable = concepts::nothrow_invocable<bind_fn, Args...>;

    template<typename... Args>
    using bind_type = ::std::invoke_result_t<bind_fn, Args...>;

    inline constexpr struct lazy_invoke_fn
    {
        template<typename... Args, ::std::invocable<Args...> Func>
            requires bindable<Func, Args...>
        [[nodiscard]] constexpr auto operator()(Func&& func, Args&&... args) const
            noexcept(nothrow_bindable<Func, Args...>)
        {
            return bind(::std::forward<Func>(func), ::std::forward<Args>(args)...);
        }
    } lazy_invoke{};

    template<typename... Args>
    concept lazy_invocable = ::std::invocable<lazy_invoke_fn, Args...>;

    template<typename... Args>
    concept nothrow_lazy_invocable = concepts::nothrow_invocable<lazy_invoke_fn, Args...>;
}