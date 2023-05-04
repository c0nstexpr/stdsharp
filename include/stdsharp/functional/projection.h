#pragma once

#include "bind.h"

namespace stdsharp
{
    inline constexpr struct projected_invoke_fn
    {
        template<typename Fn, typename Projector, typename... Args>
            requires requires //
        {
            requires(::std::invocable<Projector, Args> && ...);
            requires ::std::invocable<Fn, ::std::invoke_result_t<Projector, Args>...>;
        }
        constexpr decltype(auto) operator()(Fn&& fn, Projector projector, Args&&... args) const
            noexcept(nothrow_invocable<Fn, ::std::invoke_result_t<Projector, Args>...>)
        {
            return ::std::invoke(cpp_forward(fn), ::std::invoke(projector, cpp_forward(args))...);
        }
    } projected_invoke{};

    template<typename... Args>
    concept projected_invocable = ::std::invocable<projected_invoke_fn, Args...>;

    template<typename... Args>
    concept projected_nothrow_invocable = nothrow_invocable<projected_invoke_fn, Args...>;

    template<typename Proj>
    struct projector : value_wrapper<Proj>
    {
        using base = value_wrapper<Proj>;
        using base::base;

#define BS_OPERATOR(const_, ref)                                                         \
    template<typename Func, typename... Args>                                            \
        requires(projected_invocable<Func, const_ Proj ref, Args...>)                    \
    constexpr decltype(auto) operator()(Func&& func, Args&&... args)                     \
        const_ ref noexcept(projected_nothrow_invocable<Func, const_ Proj ref, Args...>) \
    {                                                                                    \
        return projected_invoke(                                                         \
            cpp_forward(func),                                                           \
            static_cast<const_ Proj ref>(value_wrapper<Proj>::value),                    \
            cpp_forward(args)...                                                         \
        );                                                                               \
    }

        BS_OPERATOR(, &)
        BS_OPERATOR(const, &)
        BS_OPERATOR(, &&)
        BS_OPERATOR(const, &&)

#undef BS_OPERATOR
    };

    template<typename Proj>
    projector(Proj&&) -> projector<::std::decay_t<Proj>>;

    inline constexpr struct make_projector_fn
    {
        template<typename Func>
            requires requires { projector{::std::declval<Func>()}; }
        [[nodiscard]] constexpr auto operator()(Func&& func) const
            noexcept(noexcept(projector{cpp_forward(func)}))
        {
            return projector{cpp_forward(func)};
        }
    } make_projector{};

    inline constexpr struct projected_fn
    {
        template<
            typename Proj,
            typename Func,
            std_bindable<Func> Projector =
                ::std::invoke_result_t<make_projector_fn, Proj> // clang-format off
        > // clang-format on
        constexpr auto operator()(Proj&& proj, Func&& func) const
            noexcept(nothrow_invocable<make_projector_fn, Proj>&&
                         nothrow_std_bindable<Projector, Func>)
        {
            return ::std::bind(make_projector(cpp_forward(func));
        }
    } projected{};

    template<typename... Args>
    concept projectable = ::std::invocable<projected_fn, Args...>;

    template<typename... Args>
    concept nothrow_projectable = nothrow_invocable<projected_fn, Args...>;

    template<typename... Args>
    using projected_t = ::std::invoke_result_t<projected_fn, Args...>;
}