#include "test.h"
#include "stdsharp/memory/static_allocator.h"
#include "stdsharp/memory/allocator_traits.h"
#include "stdsharp/memory/pointer_traits.h"

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
        int v = 0;

        constexpr derived(const int v): v(v) {}

        [[nodiscard]] constexpr int foo() const override { return v; }
    };

    // NOLINTBEGIN(*-reinterpret-cast)

    constexpr auto alloc_size = sizeof(derived) * 4;

    GIVEN("allocator with " << alloc_size << " bytes")
    {
        static_allocator<char, alloc_size> allocator;

        THEN("constructing a derived type at allocator")
        {
            const auto p1 = allocator.allocate(sizeof(derived));
            const auto d1_ptr = reinterpret_cast<derived*>(p1);

            {
                const base* base_ptr = d1_ptr;
                constexpr int v = 42;

                std::ranges::construct_at(d1_ptr, v);

                REQUIRE(base_ptr->foo() == v);
            }

            AND_THEN("constructing anther one at allocator")
            {
                const auto p2 = allocator.allocate(sizeof(derived));
                const auto d2_ptr = reinterpret_cast<derived*>(p2);

                {
                    constexpr int v2 = 42;
                    const base* base_ptr = d2_ptr;

                    std::ranges::construct_at(d2_ptr, v2);

                    REQUIRE(base_ptr->foo() == v2);
                }

                allocator.deallocate(p2, sizeof(derived));
            }

            allocator.deallocate(p1, sizeof(derived));
        }

        THEN("constructing two derived types at allocator")
        {
            constexpr array<derived, 2> derived_array{42, 13};

            array<derived*, 2> ptrs{};

            for(auto it = ptrs.begin(); const auto& d : derived_array)
            {
                auto& d_ptr = *it;

                d_ptr = reinterpret_cast<derived*>(allocator.allocate(sizeof(derived)));

                std::ranges::construct_at(d_ptr, d);

                const base* base_ptr = d_ptr;

                REQUIRE(base_ptr->foo() == d.foo());

                ++it;
            }

            AND_THEN("release first type and construct the third type")
            {
                allocator.deallocate(reinterpret_cast<char*>(ptrs.front()), sizeof(derived));

                constexpr derived d3{65};

                const auto ptr = allocator.allocate(sizeof(derived));
                const auto d3_ptr = reinterpret_cast<derived*>(ptr);
                const base* base3_ptr = d3_ptr;

                std::ranges::construct_at(d3_ptr, d3);

                REQUIRE(to_void_pointer(ptr) == to_void_pointer(allocator.storage().data()));
                REQUIRE(base3_ptr->foo() == d3.foo());
                REQUIRE(ptrs[1]->foo() == derived_array[1].foo());

                allocator.deallocate(ptr, sizeof(derived));
            }
        }

        THEN("constructing 5 d1 at allocator should throws")
        {
            REQUIRE_THROWS_AS(allocator.allocate(sizeof(derived) * 5), bad_alloc);
        }
    }

    GIVEN("allocator with 4 * sizeof(int), constexpr allocate and deallocate")
    {
        [[maybe_unused]] constexpr int _ = ( //
            []
            {
                static_allocator<char, 4 * sizeof(int)> allocator;

                const auto p1 = allocator.allocate(1);
                allocator.deallocate(p1, 1);
            }(),
            0
        );
    }

    // NOLINTEND(*-reinterpret-cast)
}
