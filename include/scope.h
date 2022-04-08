#pragma once
#include <exception>

#include "cstdint/cstdint.h"
#include "functional/invoke.h"
#include "type_traits/object.h"
#include "enumeration.h"
#include "utility/value_wrapper.h"

namespace stdsharp::scope
{
    using exit_fn_policy = flag< //
        decltype(
            []
            {
                enum class flag : u8
                {
                    on_success = 1,
                    on_failure = 2,
                    on_exit = on_success | on_failure
                };
                return flag{};
            }() // clang-format off
        )
    >; // clang-format on

    template<exit_fn_policy Policy, concepts::nothrow_invocable Fn>
    struct [[nodiscard]] scoped : // NOLINT(*-special-member-functions)
        private value_wrapper<Fn>,
        type_traits::unique_object
    {
    private:
        using enum_t = exit_fn_policy::enum_type;
        using unique_base = type_traits::unique_object;
        using wrapper = value_wrapper<Fn>;
        using wrapper::value;

    public:
        using exit_fn_t = Fn;
        using unique_base::unique_base;
        using wrapper::wrapper;

        static constexpr auto policy = Policy;

        constexpr ~scoped()
        {
            const auto execute = [this] noexcept { ::std::invoke(::std::move(value)); };

            if constexpr(Policy == enum_t::on_exit) execute();
            else if(::std::uncaught_exceptions() == 0)
                functional::conditional_invoke<Policy == enum_t::on_success>(execute);
            else
                functional::conditional_invoke<Policy == enum_t::on_exit>(execute);
        }
    };

    template<exit_fn_policy Policy>
    struct make_scoped_fn
    {
    private:
        template<typename Fn>
        using scoped_t = scoped<Policy, ::std::decay_t<Fn>>;

    public:
        template<typename Fn>
            requires ::std::constructible_from<scoped_t<Fn>, Fn>
        constexpr scoped_t<Fn> operator()(Fn&& fn) const
            noexcept(concepts::nothrow_constructible_from<scoped_t<Fn>, Fn>)
        {
            return ::std::forward<Fn>(fn);
        }
    };

    template<exit_fn_policy Policy>
    inline constexpr make_scoped_fn<Policy> make_scoped{};

    inline constexpr struct make_raii_scope_fn
    {
        template<::std::invocable Fn>
        constexpr decltype(auto) operator()(Fn&& fn, const auto&&...) const
            noexcept(concepts::nothrow_invocable<Fn>)
        {
            return ::std::invoke(::std::forward<Fn>(fn));
        }
    } make_raii_scope{};

    template<exit_fn_policy... Policies>
    struct make_scopes_fn
    {
        template<typename Fn, typename... ExitFn>
            requires ::std::invocable<
                make_raii_scope_fn,
                Fn,
                ::std::invoke_result_t<make_scoped_fn<Policies>, ExitFn>... // clang-format off
            > // clang-format on
        constexpr decltype(auto) operator()(Fn&& fn, ExitFn&&... exit_fn) const
            noexcept(concepts::nothrow_invocable<Fn>)
        {
            return make_raii_scope(
                ::std::forward<Fn>(fn),
                make_scoped<Policies>(::std::forward<ExitFn>(exit_fn))... //
            );
        }
    };

    template<exit_fn_policy... Policies>
    inline constexpr make_scopes_fn<Policies...> make_scopes{};
}