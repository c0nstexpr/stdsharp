#pragma once

#include "../type_traits/core_traits.h"
#include "../concepts/concepts.h"

namespace stdsharp
{
    template<expr_req ExprReq, typename Ret, typename... Args>
    struct implement_dispatcher :
        ::std::reference_wrapper<Ret(Args&&...) noexcept(ExprReq == expr_req::no_exception)>
    {
    private:
        using func = ::meta::_t<implement_dispatcher>;

    public:
        static constexpr auto requirement = ExprReq;

        using ::std::reference_wrapper<func>::reference_wrapper;

        template<invocable_r<Ret, Args...> Closure>
        constexpr implement_dispatcher(const Closure&) noexcept
            requires requires(const Closure& c) //
        {
            constant<(Closure{}, 0)>{};
            requires !(ExprReq == expr_req::no_exception) ||
                nothrow_invocable_r<Closure, Ret, Args...>;
            +c;
        }
            :
            ::std::reference_wrapper<func>(
                +[](Args&&... args) -> Ret
                {
                    constexpr Closure c{};
                    return c(cpp_forward(args)...);
                }
            )
        {
        }

        constexpr implement_dispatcher() noexcept
            requires ::std::same_as<void, Ret>
            : implement_dispatcher([](const Args&...) noexcept {})
        {
        }

        constexpr implement_dispatcher() noexcept
            requires requires //
        {
            Ret{};
            requires !(ExprReq == expr_req::no_exception) || noexcept(Ret{});
        }
            :
            implement_dispatcher( //
                [](const Args&...) noexcept(ExprReq == expr_req::no_exception) { return Ret{}; }
            )
        {
        }

        template<expr_req OtherReq, typename OtherRet>
        constexpr implement_dispatcher(
            const implement_dispatcher<OtherReq, OtherRet, Args...>& other
        ) noexcept
            requires invocable_r<decltype(other), Ret, Args...>
            : ::std::reference_wrapper<func>(other.get())
        {
        }
    };

    template<typename Ret, typename... Arg>
    struct implement_dispatcher<expr_req::ill_formed, Ret, Arg...> : empty_t
    {
        static constexpr auto requirement = expr_req::ill_formed;
    };
}