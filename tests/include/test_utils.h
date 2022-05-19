#pragma once
#include <boost/ut.hpp>
#include <fmt/ranges.h>

#include "stdsharp/type_traits/core_traits.h"

namespace stdsharp::test
{
    template<auto... V>
    using static_params = type_traits::regular_value_sequence<V...>;
}

namespace boost::inline ext::ut // NOLINT(modernize-concat-nested-namespaces)
{
    namespace details
    {
        template<typename T>
        concept ut_expectable = requires(T v)
        {
            expect(v);
        };
    }

    inline auto& get_empty_suite() noexcept
    {
        static suite s{[]() noexcept {}};
        return s;
    }

    template<auto Value>
    constexpr auto static_expect(
        const reflection::source_location& location = reflection::source_location::current() //
    )
    {
        // clang-format off
        if constexpr(details::ut_expectable<decltype(Value)>) return expect(Value, location);
        else return expect(_t{Value}, location); // clang-format on
    }

    inline auto print(const std::string_view& str) { return log << fmt::format(" \"{}\"", str); }

    inline auto println(std::string str) { return print(std::move(str) + "\n"); }
}
