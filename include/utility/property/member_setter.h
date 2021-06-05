// Created by BlurringShadow at 2021-03-11-下午 4:30

#pragma once

#include "utility/traits/member.h"

namespace blurringshadow::utility
{
    namespace details
    {
        template<auto MemberPtr, typename... Args>
        struct member_setter_constraints
        {
            using traits = traits::member_function_pointer_traits<MemberPtr>;
            using class_t = typename traits::class_t;

            static constexpr auto v = std::invocable<decltype(MemberPtr), class_t, Args...>;
        };
    }

    template<auto MemberPtr, typename... Args> 
    concept member_setter = details::member_setter_constraints<MemberPtr, Args...>::v;
}
