#include "test.h"
#include "stdsharp/memory/static_allocator.h"
#include "stdsharp/memory/allocator_traits.h"

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

    GIVEN("allocator with 16 bytes")
    {
        static_allocator<char, sizeof(d1) * 4> allocator;

        using traits = stdsharp::allocator_traits<decltype(allocator)>;

        THEN("constructing a d1 at allocator")
        {
            const auto ptr = traits::allocate(allocator, sizeof(d1));

            const auto d1_ptr = reinterpret_cast<d1*>(ptr); // NOLINT(*-reinterpret-cast)

            std::ranges::construct_at(d1_ptr);

            const base* base_ptr = d1_ptr;

            REQUIRE(base_ptr->foo() == 0);

            traits::deallocate(allocator, ptr, sizeof(d1));
        }

        THEN("constructing a d2 at allocator")
        {
            const auto ptr = traits::allocate(allocator, sizeof(d2));

            const auto d2_ptr = reinterpret_cast<d2*>(ptr); // NOLINT(*-reinterpret-cast)

            std::ranges::construct_at(d2_ptr);

            const base* base_ptr = d2_ptr;

            REQUIRE(base_ptr->foo() == 1);

            traits::deallocate(allocator, ptr, sizeof(d2));
        }

        THEN("constructing 5 d1 at allocator should throws")
        {
            REQUIRE_THROWS_AS(traits::allocate(allocator, sizeof(d2) * 5), bad_alloc);
        }
    }
}
