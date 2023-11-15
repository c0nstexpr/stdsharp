#include "stdsharp/functional/operations.h"
#include "stdsharp/functional/pipeable.h"
#include "test.h"

using namespace std;
using namespace stdsharp;

SCENARIO("pipeable, [functional]") // NOLINT
{
    {
        constexpr auto fn = make_pipeable<pipe_mode::left>(identity_v);

        STATIC_REQUIRE((1 | fn) == 1);
    }

    {
        constexpr auto fn = make_pipeable<pipe_mode::right>(identity_v);

        STATIC_REQUIRE((fn | 1) == 1);
    }
}