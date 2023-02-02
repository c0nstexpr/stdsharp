#include "stdsharp/memory/scoped_memory.h"
#include "stdsharp/memory/static_allocator.h"
#include "test_allocator.h"

SCENARIO("make raii memory", "[memory][raii_memory]") // NOLINT
{
    GIVEN("an allocator")
    {
        test_allocator<int> alloc;

        WHEN("allocate a int")
        {
            const auto mem = scope::allocate(alloc, 1);
            const auto ptr = mem.ptr();

            *ptr = 42;

            INFO(fmt::format("address: {}", to_void_pointer(ptr)));

            REQUIRE(*ptr == 42);
        }

        alloc.leak_check();
    }
}
