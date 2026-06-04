#pragma once

#include "../tuple/get.h"
#include "invocables.h"

namespace stdsharp
{
    inline constexpr struct bind_front_fn
    {
        [[nodiscard]] static constexpr auto operator()(auto&&... args) //
            noexcept(noexcept(std::bind_front(cpp_forward(args)...))) //
            -> decltype(std::bind_front(cpp_forward(args)...))
        {
            return std::bind_front(cpp_forward(args)...);
        }
    } bind_front{};

    inline constexpr struct bind_back_fn
    {
        [[nodiscard]] static constexpr auto operator()(auto&&... args) //
            noexcept(noexcept(std::bind_back(cpp_forward(args)...))) //
            -> decltype(std::bind_back(cpp_forward(args)...))
        {
            return std::bind_back(cpp_forward(args)...);
        }
    } bind_back{};

    inline constexpr struct forward_bind_front_fn
    {
        template<typename Fn, typename... Args>
        [[nodiscard]] static constexpr auto operator()(Fn&& fn, Args&&... args) noexcept
        {
            return [&args..., &fn]<typename... CallArgs>(CallArgs&&... call_args) //
                noexcept(nothrow_invocable<Fn, Args..., CallArgs...>)
                requires std::invocable<Fn, Args..., CallArgs...>
            {
                return std::
                    invoke(cpp_forward(fn), cpp_forward(args)..., cpp_forward(call_args)...);
            };
        }
    } forward_bind_front{};

    inline constexpr struct forward_bind_back_fn
    {
        template<typename Fn, typename... Args>
        [[nodiscard]] static constexpr auto operator()(Fn&& fn, Args&&... args) noexcept
        {
            return [&args..., &fn]<typename... CallArgs>(CallArgs&&... call_args) //
                noexcept(nothrow_invocable<Fn, CallArgs..., Args...>)
                requires std::invocable<Fn, CallArgs..., Args...>
            {
                return std::
                    invoke(cpp_forward(fn), cpp_forward(call_args)..., cpp_forward(args)...);
            };
        }
    } forward_bind_back{};
}
