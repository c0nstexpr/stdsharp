#include "test.h"
#include "stdsharp/memory/static_allocator.h"

using namespace stdsharp;
using namespace std;

SCENARIO("static allocator", "[memory][static_allocator]") // NOLINT
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
        static_allocator<char, sizeof(d1) * 4> buffer;

        using traits = allocator_traits<decltype(buffer)>;

        THEN("constructing a d1 at buffer")
        {
            const auto ptr = traits::allocate(buffer, sizeof(d1));

            const auto d1_ptr = reinterpret_cast<d1*>(ptr); // NOLINT(*-reinterpret-cast)

            std::ranges::construct_at(d1_ptr);

            const base* base_ptr = d1_ptr;

            REQUIRE(base_ptr->foo() == 0);
        }
    }
}
