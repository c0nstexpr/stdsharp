#pragma once
#include <nonstd/scope.hpp>

#include "concepts/concepts.h"

namespace stdsharp
{
    using nonstd::scope_exit; // NOLINT(misc-unused-using-decls)
    using nonstd::scope_success; // NOLINT(misc-unused-using-decls)
    using nonstd::scope_fail; // NOLINT(misc-unused-using-decls)
    using nonstd::make_scope_exit; // NOLINT(misc-unused-using-decls)
    using nonstd::make_scope_success; // NOLINT(misc-unused-using-decls)
    using nonstd::make_scope_fail; // NOLINT(misc-unused-using-decls)
    using nonstd::unique_resource; // NOLINT(misc-unused-using-decls)
    using nonstd::make_unique_resource_checked; // NOLINT(misc-unused-using-decls)

    template<::std::invocable Func>
    constexpr decltype(auto) scope(Func&& func, const auto&&...) //
        noexcept(concepts::nothrow_invocable<Func>)
    {
        return ::std::invoke(func);
    }
}