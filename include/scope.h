#pragma once
#include <exception>

#include "cstdint/cstdint.h"
#include "functional/invoke.h"
#include "type_traits/core_traits.h"
#include "type_traits/object.h"
#include "enumeration.h"
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

    template<concepts::nothrow_invocable Fn, exit_fn_policy Policy>
    struct [[nodiscard]] scoped :// NOLINT(hicpp-special-member-functions, cppcoreguidelines-special-member-functions)
        type_traits::unique_object
    {
    private:
        using enum_t = exit_fn_policy::enum_type;

    public:
        Fn fn;

        constexpr ~scoped()
        {
            constexpr auto execute = [this] noexcept { ::std::invoke(::std::move(fn)); };

            if constexpr(Policy == enum_t::on_exit) execute();
            else if(::std::uncaught_exceptions() == 0)
                functional::conditional_invoke<Policy == enum_t::on_success>(execute);
            else
                functional::conditional_invoke<Policy == enum_t::on_exit>(execute);
        }
    };

    template<::std::invocable Func>
    constexpr decltype(auto) make_scope(Func&& func, const auto&&...) //
        noexcept(concepts::nothrow_invocable<Func>)
    {
        return ::std::invoke(func);
    }
}