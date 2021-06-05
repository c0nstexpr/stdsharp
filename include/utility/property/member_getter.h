// Created by BlurringShadow at 2021-03-11-下午 4:42

#pragma once

#include "traits/member.h"

namespace blurringshadow::utility
{
    namespace details
    {
        template<auto MemberPtr>
        struct member_getter_constraints
        {
            using traits = traits::member_function_pointer_traits<MemberPtr>;
            using class_t = typename traits::class_t;
            using return_t = typename traits::result_t;

            static constexpr auto v = traits::args_t::size == 0;
        };
    }

    template<auto MemberPtr> 
    concept member_getter = details::member_getter_constraints<MemberPtr>::v;
}
