#include "stdsharp/memory/static_allocator.h"
#include "stdsharp/memory/composed_allocator.h"
#include "test_allocator.h"

SCENARIO("allocate memory", "[memory][composed_allocator]") // NOLINT
{
    GIVEN("an allocator tuple")
    {
        composed_allocator<static_allocator<int, 4>, test_allocator<int>> alloc;

        WHEN("allocate a int")
        {
            const auto mem = alloc.allocate(1);
            const auto ptr = mem.ptr;

            *ptr = 42;

            REQUIRE(*ptr == 42);

            alloc.deallocate(mem, 1);
        }

        constexpr auto count = 5;
        WHEN(fmt::format("allocate {} ints", count))
        {
            const auto mem = alloc.allocate(count);
            const auto ptr = mem.ptr;

            INFO(fmt::format("address: {}", to_void_pointer(ptr)));

            THEN("write values")
            {
                constexpr auto data = views::iota(0, count);
                std::ranges::copy(data, ptr);
                REQUIRE_THAT(data, Catch::Matchers::RangeEquals(span(ptr, count)));
            }

            alloc.deallocate(mem, count);

            get<1>(alloc.allocators).leak_check();
        }
    }

    GIVEN("an zero size static allocator tuple")
    {
        composed_allocator<static_allocator<int, 0>> alloc;

        WHEN("allocate a int") { REQUIRE_THROWS_AS(alloc.allocate(1), aggregate_bad_alloc<1>); }
    }
}