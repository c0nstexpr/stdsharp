#pragma once

#include "invoke.h"
#include "../utility/to_lvalue.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename, typename, typename...>
    class lvalue_binder;

    template<typename Fn, std::size_t... I, typename... Args>
    class lvalue_binder<Fn, std::index_sequence<I...>, Args...> :
        stdsharp::indexed_values<invocables<Fn>, Args...>
    {
        using invocable_t = invocables<Fn>;
        using values_t = stdsharp::indexed_values<invocable_t, Args...>;

    public:
        constexpr lvalue_binder(auto&&... args) noexcept(noexcept(values_t{cpp_forward(args)...}))
            requires requires { values_t{cpp_forward(args)...}; }
            : values_t(cpp_forward(args)...)
        {
        }

    private:
        template<
            typename This,
            typename... TArgs,
            typename Indexed = cv_ref_align_t<This&&, values_t>,
            typename Invocable = cv_ref_align_t<This&&, invocable_t>>
            requires std::invocable<Invocable, cv_ref_align_t<This&&, Args>..., TArgs...>
        static constexpr decltype(auto) operator_impl(This&& this_, TArgs&&... args) //
            noexcept(nothrow_invocable<Invocable, cv_ref_align_t<This&&, Args>..., TArgs...>)
        {
            return invoke(
                cpo::get_element<0>(static_cast<Indexed>(this_)),
                cpo::get_element<I>(static_cast<Indexed>(this_))...,
                cpp_forward(args)...
            );
        }

    public:
        STDSHARP_MEM_PACK(operator(), operator_impl, lvalue_binder)
    };

    template<typename Fn, typename... Args>
    lvalue_binder(Fn&&, Args&&...)
        -> lvalue_binder<std::decay_t<Fn>, std::index_sequence_for<Args...>, to_lvalue_t<Args>...>;
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