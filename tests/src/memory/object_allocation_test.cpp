#include "test.h"
#include "stdsharp/memory/object_allocation.h"
#include "stdsharp/memory/static_memory_resource.h"

#include <any>

using namespace stdsharp;
using namespace std;

using allocator_t = allocator<generic_storage>;

SCENARIO("object allocation basic requirements", "[memory][object allocation]") // NOLINT
{
    STATIC_REQUIRE(default_initializable<object_allocation_like<int, allocator_t>>);
    STATIC_REQUIRE(default_initializable<trivial_object_allocation<allocator_t>>);
    STATIC_REQUIRE(default_initializable<normal_object_allocation<allocator_t>>);
    STATIC_REQUIRE(default_initializable<normal_movable_object_allocation<allocator_t>>);

    STATIC_REQUIRE(nothrow_movable<normal_movable_object_allocation<allocator_t>>);
    STATIC_REQUIRE(nothrow_swappable<normal_movable_object_allocation<allocator_t>>);
    STATIC_REQUIRE(copyable<normal_object_allocation<allocator_t>>);

    using worst_allocation = basic_object_allocation<
        []
        {
            auto req = special_mem_req::ill_formed;
            req.destruct = expr_req::no_exception;
            return req;
        }(),
        allocator_t>;

    STATIC_REQUIRE(default_initializable<worst_allocation>);
    STATIC_REQUIRE(nothrow_movable<worst_allocation>);
    STATIC_REQUIRE(nothrow_swappable<worst_allocation>);
}

SCENARIO("object allocation assign value", "[memory][object allocation]") // NOLINT
{
    GIVEN("a normal object allocation")
    {
        normal_object_allocation<allocator_t> allocation;

        WHEN("emplace an int value")
        {
            auto value = allocation.emplace<int>(1);

            THEN("the return value should correct") { REQUIRE(value == 1); }
        }

        WHEN("emplace an int vector")
        {
            const auto list = {1, 2};
            const auto& value = allocation.emplace(vector<int>{list});

            THEN("the return value should correct")
            {
                REQUIRE_THAT(value, Catch::Matchers::RangeEquals(list));
            }
        }

        WHEN("assign an int value")
        {
            allocation = 1;

            THEN("the return value should correct") { REQUIRE(allocation.get<int>() == 1); }
        }
    }
}