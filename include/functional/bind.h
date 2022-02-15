#pragma once
#include <functional>

#include "utility/value_wrapper.h"
#include "type_traits/core_traits.h"

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
    class bind_t : value_wrapper<Func>, value_wrapper<T>...
    {
    public:
        template<typename Fn, typename... U>
            requires ::std::constructible_from<value_wrapper<Func>, Fn> &&
                (::std::constructible_from<value_wrapper<T>, U>&&...)
                // NOLINTNEXTLINE(hicpp-explicit-conversions)
                constexpr bind_t(Fn&& fn, U&&... u):
                value_wrapper<Func>(::std::forward<Fn>(fn)),
                value_wrapper<T>(::std::forward<U>(u))...
            {
            }

#define BS_OPERATOR(const_, ref)                                                \
    template<typename... Args>                                                  \
        requires ::std::invocable<const_ Func ref, const_ T ref..., Args...>    \
    constexpr decltype(auto) operator()(Args&&... args) const_ ref noexcept(    \
        concepts::nothrow_invocable<const_ Func ref, const_ T ref..., Args...>) \
    {                                                                           \
        return ::std::invoke(                                                   \
            static_cast<const_ Func ref>(value_wrapper<Func>::value),           \
            static_cast<const_ T ref>(value_wrapper<T>::value)...,              \
            ::std::forward<Args>(args)...);                                     \
    }

            BS_OPERATOR(, &)
            BS_OPERATOR(const, &)
            BS_OPERATOR(, &&)
            BS_OPERATOR(const, &&)

#undef BS_OPERATOR
    };

    template<typename Func, typename... Args>
    bind_t(Func&&, Args&&...) -> bind_t<::std::decay_t<Func>, type_traits::coerce_t<Args>...>;

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