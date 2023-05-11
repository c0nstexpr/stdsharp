#pragma once

#include "../type_traits/core_traits.h"
#include "../concepts/concepts.h"

namespace stdsharp
{
    template<expr_req ExprReq, typename Ret, typename... Args>
    struct implement_dispatcher :
        ::std::reference_wrapper<Ret(Args...) noexcept(ExprReq == expr_req::no_exception)>
    {
    private:
        using func = ::meta::_t<implement_dispatcher>;
        using m_base = ::std::reference_wrapper<func>;

    public:
        using m_base::m_base;

        static constexpr auto requirement = ExprReq;

        template<invocable_r<Ret, Args...> Closure>
        constexpr implement_dispatcher(const Closure) noexcept
            requires requires //
        {
            constant<(Closure{}, 0)>{};
            requires !(ExprReq == expr_req::no_exception) ||
                nothrow_invocable_r<Closure, Ret, Args...>;
        }
            :
            m_base(
                *+[](Args... args) noexcept(ExprReq == expr_req::no_exception) -> Ret
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
                [](Args...) noexcept(ExprReq == expr_req::no_exception) { return Ret{}; }
            )
        {
        }
    };

    template<typename Ret, typename... Arg>
    struct implement_dispatcher<expr_req::ill_formed, Ret, Arg...> : empty_t
    {
        static constexpr auto requirement = expr_req::ill_formed;
    };
}