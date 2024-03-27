#pragma once

#include <range/v3/functional/pipeable.hpp>

#include "invocables.h"

// port from range-v3 pipeable.hpp
// add noexcept feature
namespace stdsharp
{
    enum class pipe_mode : std::uint8_t
    {
        left,
        right
    };

    template<typename T, pipe_mode Mode = pipe_mode::left>
    struct pipeable_base
    {
        static constexpr auto pipe_mode = Mode;

    private:
        template<typename Arg, decay_same_as<T> Pipe>
            requires std::invocable<Pipe, Arg> && (Mode == pipe_mode::left)
        friend constexpr decltype(auto) operator|(Arg&& arg, Pipe&& pipe)
            noexcept(nothrow_invocable<Pipe, Arg>)
        {
            return invoke(cpp_forward(pipe), cpp_forward(arg));
        }

        template<typename Arg, decay_same_as<T> Pipe>
            requires std::invocable<Pipe, Arg> && (Mode == pipe_mode::right)
        friend constexpr decltype(auto) operator|(Pipe&& pipe, Arg&& arg)
            noexcept(nothrow_invocable<Pipe, Arg>)
        {
            return invoke(cpp_forward(pipe), cpp_forward(arg));
        }
    };
}

namespace stdsharp
{
    template<pipe_mode Mode = pipe_mode::left>
    struct make_pipeable_fn
    {
        template<typename Fn>
        struct piper : invocables<Fn>, pipeable_base<piper<Fn>, Mode>
        {
            piper() = default;

            using invocables<Fn>::invocables;
        };

        template<typename Fn>
            requires std::constructible_from<std::decay_t<Fn>, Fn>
        constexpr auto operator()(Fn&& fn) const
            noexcept(nothrow_constructible_from<std::decay_t<Fn>, Fn>)
        {
            return piper<std::decay_t<Fn>>{cpp_forward(fn)};
        }
    };

    template<pipe_mode Mode = pipe_mode::left>
    inline constexpr make_pipeable_fn<Mode> make_pipeable{};

    template<typename T, pipe_mode Mode = pipe_mode::left>
    concept pipeable = std::derived_from<std::decay_t<T>, pipeable_base<T, Mode>> ||
        (Mode == pipe_mode::left && ranges::is_pipeable_v<std::decay_t<T>>);

    template<typename Pipe>
        requires pipeable<Pipe> || pipeable<Pipe, pipe_mode::right>
    inline constexpr auto get_pipe_mode = pipeable<Pipe> ? pipe_mode::left : pipe_mode::right;
}