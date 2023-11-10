#include "stdsharp/filesystem/filesystem.h"
#include "test.h"

using namespace std;
using namespace stdsharp;

using namespace stdsharp::filesystem;

SCENARIO("space size", "[filesystem]") // NOLINT
{
    using bytes = stdsharp::filesystem::bytes;

    STATIC_REQUIRE(default_initializable<bytes>);
    STATIC_REQUIRE(invocable<plus<>, bytes, bytes>);

    STATIC_REQUIRE( //
        requires(bytes v) //
        {
            +v;
            -v;
        }
    );

    STATIC_REQUIRE((1_bit + 1_bit) == 2_bit);

    {
        constexpr auto v = 1'000'042_KB;

        REQUIRE(format("{:-<5MB}", v) == "1000MB42KB");
        REQUIRE(format("{:-<5.1GB}", v) == "1GB--");
        REQUIRE(format("{:->5.1GB}", v) == "--1GB");
        REQUIRE(format("{:-<5GB}", v) == "1GB0MB42KB");
        REQUIRE(format("{:-^5.1GB}", v) == "-1GB-");
        REQUIRE(format("{:.4GB}", v) == "1GB0MB42KB0B");
        REQUIRE(format("{}", 1.2_GB) == "1.2GB");
    }
};