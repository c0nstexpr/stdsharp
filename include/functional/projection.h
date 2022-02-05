#pragma once
#include "utility/value_wrapper.h"
#include "functional/invocables.h"

namespace stdsharp::functional
{
    inline constexpr auto projected_invoke = []<typename Fn, typename Projector, typename... Args>
        requires(::std::invocable<Projector&, Args>&&...)
    &&::std::invocable<Fn, ::std::invoke_result_t<Projector, Args>...>(
          Fn&& fn,
          Projector projector,
          Args&&... args // clang-format off
    ) noexcept(concepts::nothrow_invocable<Fn, ::std::invoke_result_t<Projector, Args>...>) // clang-format on
          ->decltype(auto)
    {
        return ::std::invoke(
            ::std::forward<Fn>(fn), //
            ::std::invoke(projector, ::std::forward<Args>(args))... //
        );
    };

    template<typename Fn, typename Projector, typename... Args>
    concept projected_invocable = ::std::invocable<Fn, Projector, Args...>;

    template<typename Fn, typename Projector, typename... Args>
    concept projected_nothrow_invocable = concepts::nothrow_invocable<Fn, Projector, Args...>;

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
            ::std::forward<Func>(func),                                                  \
            static_cast<const_ Proj ref>(value_wrapper<Proj>::value),                    \
            ::std::forward<Args>(args)...);                                              \
    }

        BS_OPERATOR(, &)
        BS_OPERATOR(const, &)
        BS_OPERATOR(, &&)
        BS_OPERATOR(const, &&)

#undef BS_OPERATOR
    };

    inline constexpr auto make_projector = make_trivial_invocables(
        nodiscard_tag,
        []<typename Func>(Func&& func) //
        noexcept(noexcept(projector{::std::forward<Func>(func)}))
        {
            return projector{::std::forward<Func>(func)}; //
        } //
    );
}