#include "stdsharp/functional/pipeable.h"
#include "test.h"

SCENARIO("pipeable, [functional][pipeable]") // NOLINT
{
    {
        constexpr auto fn = make_pipeable<pipe_mode::left>(std::identity{});

        fn(1);

        STATIC_REQUIRE((1 | fn) == 1);
    }

    {
        constexpr auto fn = make_pipeable<pipe_mode::right>(std::identity{});

        STATIC_REQUIRE((fn | 1) == 1);
    }
}