#include "stdsharp/memory/static_allocator.h"
#include "stdsharp/memory/composed_allocator.h"
#include "stdsharp/memory/allocator_reference.h"
#include "test_allocator.h"

SCENARIO("allocate memory", "[memory][composed_allocator]") // NOLINT
{
    GIVEN("an allocator tuple")
    {
        static_memory_resource_for<int, 4> rsc;
        test_allocator<int> test_alloc;

        composed_allocator alloc{
            static_allocator_for<int, 4>{rsc},
            allocator_reference{test_alloc} //
        };

        WHEN("allocate a int")
        {
            const auto ptr = alloc.allocate(1);

            *ptr = 42;

            REQUIRE(*ptr == 42);

            alloc.deallocate(ptr, 1);
        }

        constexpr auto count = 5;
        WHEN(format("allocate {} ints", count))
        {
            const auto ptr = alloc.allocate(count);

            INFO(format("address: {}", to_void_pointer(ptr)));

            THEN("write values")
            {
                constexpr auto data = std::views::iota(0, count);
                std::ranges::copy(data, ptr);
                REQUIRE_THAT(data, Catch::Matchers::RangeEquals(span(ptr, count)));
            }

            alloc.deallocate(ptr, count);

            test_alloc.leak_check();
        }
    }
}