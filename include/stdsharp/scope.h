#pragma once

#include <optional>

#include "enumeration.h"
#include "type_traits/object.h"

namespace stdsharp::scope
{
    enum class exit_fn_policy : unsigned char
    {
        on_success = 0b01,
        on_failure = 0b10,
        on_exit = on_success | on_failure
    };

    template<flag<exit_fn_policy> Policy, nothrow_invocable Fn>
    struct [[nodiscard]] scoped : // NOLINT(*-special-member-functions)
        private ::std::optional<Fn>,
        unique_object
    {
    private:
        constexpr void execute() noexcept
        {
            ::std::invoke(::std::move(this->value()));
            this->reset();
        };

    public:
        using exit_fn_t = Fn;

        template<typename... Args>
            requires ::std::constructible_from<Fn, Args...>
        constexpr explicit scoped(Args&&... args) noexcept(nothrow_constructible_from<Fn, Args...>):
            ::std::optional<Fn>(::std::in_place, ::std::forward<Args>(args)...)
        {
        }

        static constexpr auto policy = Policy;

        constexpr ~scoped()
        {
            if(!this->has_value()) return;

            if constexpr(policy == exit_fn_policy::on_exit) execute();
            else if(::std::is_constant_evaluated() || ::std::uncaught_exceptions() == 0)
            {
                if constexpr(policy.contains(exit_fn_policy::on_success)) execute();
            }
            else
            {
                if constexpr(policy.contains(exit_fn_policy::on_failure)) execute();
            }
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
            noexcept(nothrow_constructible_from<scoped_t<Fn>, Fn>)
        {
            return ::std::forward<Fn>(fn);
        }
    };

    template<exit_fn_policy Policy>
    inline constexpr make_scoped_fn<Policy> make_scoped{};
}