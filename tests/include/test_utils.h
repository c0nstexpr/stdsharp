#pragma once
#include <boost/ut.hpp>
#include <fmt/ranges.h>

namespace std_sharp::test::utility
{
    template<auto...>
    struct static_params
    {
    };
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
    inline constexpr auto static_expect = []
    {
        // clang-format off
        if constexpr(details::ut_expectable<decltype(Value)>) return expect(Value);
        else return expect(_t{Value}); // clang-format on
    };

    inline auto print(const std::string_view& str) { return log << fmt::format(" \"{}\"", str); }

    inline auto println(std::string str) { return print(std::move(str) + "\n"); }
}
