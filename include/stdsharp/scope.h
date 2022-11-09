#pragma once

#include <exception>

#include "cstdint/cstdint.h"
#include "functional/invoke.h"
#include "type_traits/object.h"
#include "utility/value_wrapper.h"

namespace stdsharp::scope
{
    enum class exit_fn_policy : u8
    {
        on_success = 1,
        on_failure = 2,
        on_exit = on_success | on_failure
    };

    template<exit_fn_policy Policy, nothrow_invocable Fn>
    struct [[nodiscard]] scoped : // NOLINT(*-special-member-functions)
        private value_wrapper<Fn>,
        unique_object
    {
    private:
        using unique_base = unique_object;
        using wrapper = value_wrapper<Fn>;
        using wrapper::value;

    public:
        using exit_fn_t = Fn;
        using unique_base::unique_base;
        using wrapper::wrapper;

        static constexpr auto policy = Policy;

        constexpr ~scoped()
        {
            const auto execute = [this]() noexcept { ::std::invoke(::std::move(value)); };

            if constexpr(policy == exit_fn_policy::on_exit) execute();
            else if(::std::uncaught_exceptions() == 0)
                conditional_invoke<policy == exit_fn_policy::on_success>(execute);
            else conditional_invoke<policy == exit_fn_policy::on_exit>(execute);
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