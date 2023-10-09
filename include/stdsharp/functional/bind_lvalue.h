#pragma once

#include "invoke.h"
#include "../utility/to_lvalue.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename, typename, typename...>
    class lvalue_binder;

    template<typename Fn, std::size_t... I, typename... Args>
    class lvalue_binder<Fn, std::index_sequence<I...>, Args...>
    {
    private:
        using invocable_t = stdsharp::invocables<Fn>;
        using args_t = stdsharp::indexed_values<Args...>;

        invocable_t func_;
        args_t args_;

    public:
        template<typename TFn, typename... TArgs>
            requires std::constructible_from<invocable_t, TFn> &&
                         std::constructible_from<args_t, TArgs...>
        constexpr lvalue_binder(TFn&& func, TArgs&&... args) //
            noexcept(nothrow_constructible_from<invocable_t, TFn> && nothrow_constructible_from<args_t, TArgs...>):
            func_{cpp_forward(func)}, args_{cpp_forward(args)...}
        {
        }

    private:
        template<
            typename This,
            typename... TArgs,
            typename Invocable = cv_ref_align_t<This&&, invocable_t>>
            requires std::invocable<Invocable, cv_ref_align_t<This&&, Args>..., TArgs...>
        static constexpr decltype(auto) operator_impl(This&& this_, TArgs&&... args) //
            noexcept(nothrow_invocable<Invocable, cv_ref_align_t<This&&, Args>..., TArgs...>)
        {
            return (cpp_forward(this_).func_)(
                cpo::get_element<I>(cpp_forward(this_).args_)...,
                cpp_forward(args)...
            );
        }

    public:
        STDSHARP_MEM_PACK(operator(), operator_impl, lvalue_binder)
    };

    template<typename Fn, typename... Args>
    lvalue_binder(Fn&&, Args&&...)
        -> lvalue_binder<Fn, std::index_sequence_for<Args...>, to_lvalue_t<Args>...>;
}

namespace stdsharp
{
    inline constexpr struct bind_lvalue_fn
    {
        template<typename Func, typename... Args>
        constexpr auto operator()(Func&& func, Args&&... args) const
            noexcept(noexcept(details::lvalue_binder{cpp_forward(func), cpp_forward(args)...}))
            requires requires {
                details::lvalue_binder{cpp_forward(func), cpp_forward(args)...};
            }
        {
            return details::lvalue_binder{cpp_forward(func), cpp_forward(args)...};
        }
    } bind_lvalue{};
}

#include "../compilation_config_out.h"