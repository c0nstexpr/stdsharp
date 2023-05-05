#pragma once

#include "../type_traits/core_traits.h"
#include "../concepts/concepts.h"

namespace stdsharp
{
    template<expr_req ExprReq, typename Ret, typename... Arg>
    struct implement_dispatcher :
        ::std::reference_wrapper<Ret(Arg&&...) noexcept(ExprReq == expr_req::no_exception)>
    {
    private:
        using func = ::meta::_t<implement_dispatcher>;

    public:
        static constexpr auto requirement = ExprReq;

        using ::std::reference_wrapper<func>::reference_wrapper;

        template<nothrow_convertible_to<func*> Closure>
        constexpr implement_dispatcher(Closure&& closure) noexcept:
            ::std::reference_wrapper<func>(*static_cast<func*>(closure))
        {
        }

        constexpr implement_dispatcher() noexcept
            requires ::std::same_as<void, Ret>
            : implement_dispatcher([](const Arg&...) noexcept {})
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
                [](const Arg&...) noexcept(ExprReq == expr_req::no_exception) { return Ret{}; }
            )
        {
        }

        template<expr_req OtherReq, typename OtherRet>
        constexpr implement_dispatcher(const implement_dispatcher<OtherReq, OtherRet, Arg...>& other
        ) noexcept
            requires invocable_r<decltype(other), Ret, Arg...>
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