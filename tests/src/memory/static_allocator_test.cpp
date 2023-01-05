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

    struct derived : base
    {
        int v{};

        [[nodiscard]] constexpr int foo() const override { return v; }
    };

    GIVEN("allocator with 16 bytes")
    {
        static_allocator<char, sizeof(derived) * 4> allocator;

        using traits = stdsharp::allocator_traits<decltype(allocator)>;

        THEN("constructing a derived type at allocator")
        {
            const auto ptr = traits::allocate(allocator, sizeof(derived));

            const auto d1_ptr = reinterpret_cast<derived*>(ptr); // NOLINT(*-reinterpret-cast)

            std::ranges::construct_at(d1_ptr);

            const base* base_ptr = d1_ptr;

            REQUIRE(base_ptr->foo() == 0);

            AND_THEN("constructing anther one at allocator")
            {
                const auto ptr = traits::allocate(allocator, sizeof(derived));

                const auto d2_ptr = reinterpret_cast<derived*>(ptr); // NOLINT(*-reinterpret-cast)

                std::ranges::construct_at(d2_ptr);

                const base* base_ptr = d2_ptr;

                REQUIRE(base_ptr->foo() == 1);

                traits::deallocate(allocator, ptr, sizeof(derived));
            }

            traits::deallocate(allocator, ptr, sizeof(derived));
        }

        THEN("constructing 5 d1 at allocator should throws")
        {
            REQUIRE_THROWS_AS(traits::allocate(allocator, sizeof(derived) * 5), bad_alloc);
        }
    }
}
