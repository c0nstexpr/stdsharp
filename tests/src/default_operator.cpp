#include "stdsharp/default_operator.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

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
void commutative_arithmetic_test()
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
    STATIC_REQUIRE(requires { +v; });
}

SCENARIO("incrementable", "[default operator]")
{
    [[maybe_unused]] struct : increase
    {
        using increase::operator++;
        using increase::operator--;

        auto& operator++() { return *this; }

        auto& operator--() { return *this; }
    } v{};

    STATIC_REQUIRE(requires { v++; });
    STATIC_REQUIRE(requires { v--; });
}

struct arith : arithmetic
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

SCENARIO("arithmetic", "[default operator]") { arithmetic_test<arith>(); }

struct comm :
    arith,
    unary_plus,
    plus_commutative,
    minus_commutative,
    multiply_commutative,
    divide_commutative,
    modulus_commutative,
    bitwise_and_commutative,
    bitwise_or_commutative,
    bitwise_xor_commutative,
    bitwise_left_shift_commutative,
    bitwise_right_shift_commutative
{
    using arith::operator+;
    using unary_plus::operator+;
};

SCENARIO("commutative arithmetic", "[default operator]")
{
    arithmetic_test<comm>();
    commutative_arithmetic_test<comm>();
}

SCENARIO("commutative arithmetic with int", "[default operator]")
{
    struct t : comm
    {
        int v;
    };

    arithmetic_test<t>();
    commutative_arithmetic_test<t>();
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

// TODO: multidimensional subscript
#if __cpp_multidimensional_subscript >= 202110L
SCENARIO("subscript", "[default operator]")
{
    [[maybe_unused]] struct : subscript
    {
        using subscript::operator[];

        [[nodiscard]] auto operator[](const int /*unused*/) const { return *this; }
    } v{};

    STATIC_REQUIRE(requires { v[0, 1]; });
}
#endif