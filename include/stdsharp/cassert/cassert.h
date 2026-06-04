#pragma once
#include "../concepts/object.h"
#include "../functional/invoke.h"

#include <gsl/assert>

#include <cassert>

namespace stdsharp
{
    inline constexpr auto is_debug =
#ifdef NDEBUG
        false
#else
        true
#endif
        ;

    inline constexpr auto assert_with = //
        []<typename... Args>(std::predicate<Args...> auto&& fn, Args&&... args) static noexcept
    {
        template<typename... Args>
        static void operator()(std::predicate<Args...> auto&& fn, Args&&... args) noexcept
        {
            if constexpr(is_debug) Expects(invoke(fn, cpp_forward(args)...));
        };
    } assert_with;

    inline constexpr auto assert_equal = []<typename T, typename U>(T&& t, U&& u) static noexcept
        requires std::invocable<decltype(assert_with), std::ranges::equal_to, T, U>
    {
        assert_with(std::ranges::equal_to{}, cpp_forward(t), cpp_forward(u)); //
    };

    inline constexpr auto assert_not_equal = //
        []<typename T, typename U>(T&& t, U&& u) static noexcept
        requires std::invocable<decltype(assert_with), std::ranges::not_equal_to, T, U>
    {
        assert_with(std::ranges::not_equal_to{}, cpp_forward(t), cpp_forward(u)); //
    };

    inline constexpr auto assert_not_null = //
        [](const nullable_pointer auto& ptr) static noexcept { assert_not_equal(ptr, nullptr); };
}
