#include "stdsharp/filesystem/space_size.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;
using namespace stdsharp::filesystem;

SCENARIO("space size", "[filesystem][space size]")
{
    using bytes = stdsharp::filesystem::bytes;

    STATIC_REQUIRE(default_initializable<bytes>);
    STATIC_REQUIRE(invocable<plus<>, bytes, bytes>);
    STATIC_REQUIRE(invocable<minus<>, bytes, bytes>);

    STATIC_REQUIRE((1_bits + 1_bits) == 2_bits);

    // REQUIRE(format("{}", 42_KB) == "42KB");
    // REQUIRE(format("{:-^18.2{1}}", 1.2_GB, "GigaBytes") == "--1.20GigaBytes--");

    int v = 42;
    std::print("{}", std::vformat("{}", make_format_args<format_context, int>(v)));
};