#pragma once
#include "../functional/invocables.h"

namespace stdsharp::details
{
    template<typename Fn, typename T = std::invoke_result_t<Fn>>
    struct defer_invoker : stdsharp::invocables<Fn>
    {
        using invocables<Fn>::invocables;

    private:
        static constexpr T impl(auto&& this_) noexcept(nothrow_invocable_r<Fn, T>)
        {
            return std::invoke_r<T>(cpp_forward(this_));
        }

    public:
#define STDSHARP_OPERATOR(cv, ref)                                            \
    constexpr operator T() cv ref noexcept(nothrow_invocable_r<cv Fn ref, T>) \
        requires invocable_r<cv Fn ref, T>                                    \
    {                                                                         \
        return std::invoke_r<T>(static_cast<cv Fn ref>(*this));               \
    }

        STDSHARP_OPERATOR(const volatile, &)
        STDSHARP_OPERATOR(const volatile, &&)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(const, &&)
        STDSHARP_OPERATOR(volatile, &)
        STDSHARP_OPERATOR(volatile, &&)
        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename Fn>
    defer_invoker(Fn&&) -> defer_invoker<std::decay_t<Fn>>;
}

namespace stdsharp
{
    template<typename T, typename Fn>
    constexpr details::defer_invoker<Fn, T> defer(Fn&& fn) //
        noexcept(nothrow_constructible_from<details::defer_invoker<Fn, T>, Fn>)
        requires std::constructible_from<details::defer_invoker<Fn, T>, Fn>
    {
        return {cpp_forward(fn)};
    }

    template<typename Fn>
    constexpr auto defer(Fn&& fn) noexcept(noexcept(details::defer_invoker{cpp_forward(fn)}))
        requires requires { details::defer_invoker{cpp_forward(fn)}; }
    {
        return details::defer_invoker{cpp_forward(fn)};
    }
}