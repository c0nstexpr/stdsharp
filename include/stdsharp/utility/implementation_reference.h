#pragma once

#include "../type_traits/core_traits.h"
#include "../concepts/concepts.h"

namespace stdsharp
{
    template<expr_req ExprReq, typename Ret, typename... Arg>
    struct implementation_reference :
        ::std::reference_wrapper<Ret(Arg&&...) noexcept(ExprReq == expr_req::no_exception)>
    {
    private:
        using func = typename implementation_reference::type;

    public:
        static constexpr auto requirement = ExprReq;

        constexpr implementation_reference() noexcept
            requires ::std::same_as<void, Ret>
            : ::std::reference_wrapper<func>(*+[](const Arg&...) noexcept {})
        {
        }

        constexpr implementation_reference() noexcept
            requires requires //
        {
            Ret{};
            requires !(ExprReq == expr_req::no_exception) || noexcept(Ret{});
        }
            :
            ::std::reference_wrapper<func>(
                *+[](const Arg&...) noexcept(ExprReq == expr_req::no_exception) { return Ret{}; }
            )
        {
        }

        template<expr_req OtherReq, typename OtherRet>
        constexpr implementation_reference(
            const implementation_reference<OtherReq, OtherRet, Arg...>& other
        ) noexcept
            requires invocable_r<decltype(other), Ret, Arg...>
            : ::std::reference_wrapper<func>(other.get())
        {
        }
    };

    template<typename Ret, typename... Arg>
    struct implementation_reference<expr_req::ill_formed, Ret, Arg...> : empty_t
    {
        static constexpr auto requirement = expr_req::ill_formed;
    };
}