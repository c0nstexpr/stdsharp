#pragma once

#include "../type_traits/indexed_traits.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<std::size_t Size>
    struct compose_impl
    {
        template<
            std::size_t I,
            typename... Arg,
            typename Fns,
            typename Res = std::invoke_result_t<get_element_t<I, Fns>, Arg...>>
        constexpr decltype(auto) operator()(const index_constant<I>, Fns && fn, Arg&&... arg) const
            noexcept(nothrow_invocable<compose_impl, index_constant<I + 1>, Fns, Res>)
            requires std::invocable<compose_impl, index_constant<I + 1>, Fns, Res>
        {
            return (*this)(
                index_constant<I + 1>{},
                cpp_forward(fn),
                std::invoke(cpo::get_element<I>(cpp_forward(fn)), cpp_forward(arg)...)
            );
        }

        constexpr decltype(auto) operator()(const index_constant<Size>, auto&&, auto&& arg) const noexcept
        {
            return cpp_forward(arg);
        }
    };
}

namespace stdsharp
{
    template<typename... T>
    struct composed : indexed_values<T...>
    {
    private:
        using indexed_t = indexed_values<T...>;

    public:
        using indexed_t::indexed_t;

    private:
        static constexpr auto size = std::tuple_size_v<indexed_t>;

        using compose_impl = details::compose_impl<size>;

        template<
            typename This,
            typename... Args,
            typename Constant = index_constant<0>,
            typename Indexed = cv_ref_align_t<This&&, indexed_t>>
            requires std::invocable<compose_impl, Constant, Indexed, Args...>
        static constexpr decltype(auto) operator_impl(This&& this_, Args&&... args) //
            noexcept(nothrow_invocable<compose_impl, Constant, Indexed, Args...>)
        {
            return compose_impl{}(Constant{}, static_cast<Indexed>(this_), cpp_forward(args)...);
        }

    public:
        STDSHARP_MEM_PACK(operator(), operator_impl, composed)
    };

    template<typename... T>
    composed(T&&...) -> composed<std::decay_t<T>...>;
}

#include "../compilation_config_out.h"