#include "stdsharp/utility/forward_cast.h"
#include "test.h"

using namespace std;
using namespace stdsharp;

struct t0
{
};

struct t1 : t0
{
    int v;
};

class t2 : t1
{
    float v;
};

SCENARIO("forward cast", "[utility]")
{
    GIVEN("t0")
    {
        t0 v{};

        STATIC_REQUIRE(same_as<decltype(forward_cast<t0&, t0>(v)), t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t0&, t0>(v)), const t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t0&, t0&>(v)), const t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<t0&&, t0>(v)), t0&&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t0&&, t0>(v)), const t0&&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t0&&, t0&>(v)), const t0&&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t0&&, t0&&>(v)), const t0&&>);
    }

    GIVEN("t1")
    {
        t1 v{};

        STATIC_REQUIRE(same_as<decltype(forward_cast<t1&, t0>(v)), t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t1&, t0>(v)), const t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<t1&&, t0>(v)), t0&&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t1&&, t0>(v)), const t0&&>);
    }

    GIVEN("t2")
    {
        t2 v{};

        STATIC_REQUIRE(same_as<decltype(forward_cast<t2&, t0>(v)), t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t2&, t0>(v)), const t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<t2&&, t0>(v)), t0&&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t2&&, t0>(v)), const t0&&>);
    }
}