#include "test.h"
#include "stdsharp/memory/single_static_buffer.h"

using namespace stdsharp;

SCENARIO("static buffer", "[memory][static_buffer]") // NOLINT
{
    struct base // NOLINT(*-special-member-functions)
    {
        virtual ~base() = default;
        constexpr virtual int foo() = 0;
    };

    struct d1 : base
    {
        constexpr int foo() override { return 0; }
    };

    struct d2 : base
    {
        constexpr int foo() override { return 1; }
    };

    GIVEN("buffer with 16 bytes")
    {
        single_static_buffer<sizeof(d1)> buffer;

        THEN("constructing a d1 at buffer")
        {
            const auto& proxy = buffer.construct<d1>();
            base* ptr = proxy.ptr();

            REQUIRE(ptr->foo() == 0);
        }

        THEN("constructing a d2 at buffer")
        {
            const auto& proxy = buffer.construct<d2>();
            base* ptr = proxy.ptr();

            REQUIRE(ptr->foo() == 1);
        }
    }
}
