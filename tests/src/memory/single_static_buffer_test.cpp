#include "test.h"
#include "stdsharp/memory/single_static_buffer.h"

using namespace stdsharp;

SCENARIO("static buffer", "[memory][static_buffer]") // NOLINT
{
    struct base // NOLINT(*-special-member-functions)
    {
        virtual ~base() = default;
        [[nodiscard]] constexpr virtual int foo() const = 0;
    };

    struct d1 : base
    {
        [[nodiscard]] constexpr int foo() const override { return 0; }
    };

    struct d2 : base
    {
        [[nodiscard]] constexpr int foo() const override { return 1; }
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
