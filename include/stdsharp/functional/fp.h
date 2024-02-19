#pragma once

#include "invocables.h"

#include "../compilation_config_in.h"

namespace stdsharp::fp
{
    template<typename... Fn>
    class function
    {
    private:
        invocables<Fn...> fn_;

        template<std::size_t Layer, typename... Args>
        constexpr decltype(auto) impl(Args&&... args) const
        {
            return invoke_at<Layer>(fn_, cpp_forward(args)...);
        }
    };
}

#include "../compilation_config_out.h"