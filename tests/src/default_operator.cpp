#include "stdsharp/default_operator.h"
#include "test.h"

using namespace default_operator;

template<typename T>
void arithmetic_test()
{
    [[maybe_unused]] T v{};

    STATIC_REQUIRE(requires { v + 1; });
    STATIC_REQUIRE(requires { v - 1; });
    STATIC_REQUIRE(requires { v * 1; });
    STATIC_REQUIRE(requires { v / 1; });
    STATIC_REQUIRE(requires { v % 1; });
    STATIC_REQUIRE(requires { v & 1; });
    STATIC_REQUIRE(requires { v | 1; });
    STATIC_REQUIRE(requires { v ^ 1; });
    STATIC_REQUIRE(requires { v << 1; });
    STATIC_REQUIRE(requires { v >> 1; });
}

template<typename T>
void inverse_arithmetic_test()
{
    [[maybe_unused]] T v{};

    STATIC_REQUIRE(requires { 1 + v; });
    STATIC_REQUIRE(requires { 1 - v; });
    STATIC_REQUIRE(requires { 1 * v; });
    STATIC_REQUIRE(requires { 1 / v; });
    STATIC_REQUIRE(requires { 1 % v; });
    STATIC_REQUIRE(requires { 1 & v; });
    STATIC_REQUIRE(requires { 1 | v; });
    STATIC_REQUIRE(requires { 1 ^ v; });
    STATIC_REQUIRE(requires { 1 << v; });
    STATIC_REQUIRE(requires { 1 >> v; });
    STATIC_REQUIRE(requires { -v; });
    STATIC_REQUIRE(requires { +v; });
}

SCENARIO("incrementable", "[default operator]")
{
    [[maybe_unused]] struct : increase
    {
        auto& operator++() { return *this; }

        auto& operator--() { return *this; }
    } v{};

    STATIC_REQUIRE(requires { v++; });
    STATIC_REQUIRE(requires { v--; });
}

struct arith
{
    auto& operator+=(int /*unused*/) { return *this; }

    auto& operator-=(int /*unused*/) { return *this; }

    auto& operator*=(int /*unused*/) { return *this; }

    auto& operator/=(int /*unused*/) { return *this; }

    auto& operator%=(int /*unused*/) { return *this; }

    auto& operator&=(int /*unused*/) { return *this; }

    auto& operator|=(int /*unused*/) { return *this; }

    auto& operator^=(int /*unused*/) { return *this; }

    auto& operator<<=(int /*unused*/) { return *this; }

    auto& operator>>=(int /*unused*/) { return *this; }
};

SCENARIO("arithmetic", "[default operator]")
{
    struct t : arith, arithmetic
    {
    };

    arithmetic_test<t>();
}

SCENARIO("inverse arithmetic", "[default operator]")
{
    struct t : arith, inverse_arithmetic
    {
    };

    arithmetic_test<t>();
    inverse_arithmetic_test<t>();
}

SCENARIO("inverse arithmetic with int", "[default operator]")
{
    struct t : arith, inverse_arithmetic
    {
        int v;
    };

    arithmetic_test<t>();
    inverse_arithmetic_test<t>();
}

SCENARIO("arrow", "[default operator]")
{
    struct t0
    {
        int v;
    };

    [[maybe_unused]] struct t1 : arrow
    {
        t0* p;

        [[nodiscard]] auto& operator*() const { return *p; }
    } v{};

    [[maybe_unused]] auto mem_ptr = &t0::v;

    STATIC_REQUIRE(requires { v->v; });
    STATIC_REQUIRE(requires { v->*mem_ptr; });
}

SCENARIO("subscript", "[default operator]")
{
    [[maybe_unused]] struct : subscript
    {
        using subscript::operator[];

        [[nodiscard]] auto operator[](const int /*unused*/) const { return *this; }
    } v{};

#if __cpp_multidimensional_subscript >= 202110L
    STATIC_REQUIRE(requires { v[0, 1]; });
#endif
}