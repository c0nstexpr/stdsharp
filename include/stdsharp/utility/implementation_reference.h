#pragma once

#include "../type_traits/core_traits.h"

namespace stdsharp
{
    template<expr_req ExprReq, typename Ret, typename... Arg>
    struct implementation_reference :
        ::std::reference_wrapper<Ret(Arg&&...) noexcept(ExprReq == expr_req::no_exception)>
    {
        static constexpr auto requirement = ExprReq;
    };

    template<typename Ret, typename... Arg>
    struct implementation_reference<expr_req::ill_formed, Ret, Arg...> : empty_t
    {
        static constexpr auto requirement = expr_req::ill_formed;
    };

}