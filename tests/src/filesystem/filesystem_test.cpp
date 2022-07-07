#include "stdsharp/filesystem/filesystem.h"
#include "test.h"

using namespace stdsharp::filesystem;

#if __cpp_lib_format >= 201907L
    #define FORMAT_NS ::std
#else
    #define FORMAT_NS ::fmt
#endif

SCENARIO("space size", "[filesystem]") // NOLINT
{
    using bytes = stdsharp::filesystem::bytes;

    STATIC_REQUIRE(default_initializable<bytes>);
    STATIC_REQUIRE(invocable<::std::plus<>, bytes, int>);

    STATIC_REQUIRE(::std::invocable<::std::plus<>, int, bytes>);
    STATIC_REQUIRE(::std::invocable<::std::plus<>, bytes, bytes>);

    STATIC_REQUIRE( //
        requires(bytes v) //
        {
            +v;
            -v;
            ~v;
        } //
    );

    STATIC_REQUIRE(1_bit + 1_bit == 2_bit);

    {
        constexpr auto v = 1'000'042_KB;

        REQUIRE(FORMAT_NS::format("{:-<5MB}", v) == "1000MB42KB");
        REQUIRE(FORMAT_NS::format("{:-<5.1GB}", v) == "1GB--");
        REQUIRE(FORMAT_NS::format("{:->5.1GB}", v) == "--1GB");
        REQUIRE(FORMAT_NS::format("{:-<5GB}", v) == "1GB0MB42KB");
        REQUIRE(FORMAT_NS::format("{:-^5.1GB}", v) == "-1GB-");
        REQUIRE(FORMAT_NS::format("{:.4GB}", v) == "1GB0MB42KB0B");
        REQUIRE(FORMAT_NS::format("{}", 1.2_GB) == "1.2GB");
    }
};