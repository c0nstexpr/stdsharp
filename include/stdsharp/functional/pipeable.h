#pragma once

#include <range/v3/functional/pipeable.hpp>

#include "compose.h"
#include "invocables.h"
#include "../type_traits/object.h"

// comes from range-v3 pipeable.hpp
// add noexcept feature
namespace stdsharp
{
    enum class pipe_mode
    {
        left,
        right
    };

    template<pipe_mode = pipe_mode::left>
    struct pipeable_base;

    template<pipe_mode Mode = pipe_mode::left>
    struct make_pipeable_fn
    {
        template<typename Fn>
            requires requires //
        {
            requires ::std::invocable<make_invocables_fn, Fn>;
            requires ::std::invocable<
                make_inherited_fn,
                pipeable_base<Mode>,
                ::std::invoke_result_t<make_invocables_fn, Fn> // clang-format off
            >; // clang-format on
        }
        constexpr auto operator()(Fn&& fun) const noexcept( //
            nothrow_invocable<
                make_inherited_fn,
                pipeable_base<Mode>,
                ::std::invoke_result_t<make_invocables_fn, Fn> // clang-format off
            > // clang-format on
        )
        {
            return make_inherited(pipeable_base<Mode>{}, make_invocables(cpp_forward(fun)));
        }
    };

    template<pipe_mode Mode = pipe_mode::left>
    inline constexpr make_pipeable_fn<Mode> make_pipeable{};

    template<typename T, pipe_mode Mode = pipe_mode::left>
    concept pipeable = ::std::derived_from<::std::decay_t<T>, pipeable_base<Mode>> || //
        (Mode == pipe_mode::left && ::ranges::is_pipeable_v<::std::decay_t<T>>);

    template<typename Pipe>
        requires pipeable<Pipe> || pipeable<Pipe, pipe_mode::right>
    inline constexpr auto get_pipe_mode = pipeable<Pipe> ? pipe_mode::left : pipe_mode::right;

    namespace details
    {
        class pipeable_operator
        {
            template<typename Arg, pipeable<pipe_mode::left> Pipe>
                requires ::std::invocable<Pipe, Arg>
            friend constexpr decltype(auto) operator|(Arg&& arg, Pipe&& pipe) //
                noexcept(nothrow_invocable<Pipe, Arg>)
            {
                return ::std::invoke(cpp_forward(arg));
            }

            template<pipeable<pipe_mode::right> Pipe, typename Arg>
                requires ::std::invocable<Pipe, Arg>
            friend constexpr decltype(auto) operator|(Pipe&& pipe, Arg&& arg) //
                noexcept(nothrow_invocable<Pipe, Arg>)
            {
                return ::std::invoke(cpp_forward(arg));
            }
        };
    }

    template<pipe_mode Mode>
    struct pipeable_base : details::pipeable_operator
    {
        static constexpr auto pipe_mode = Mode;
    };
}