#include "test.h"
#include "stdsharp/memory/object_allocation.h"
#include "stdsharp/memory/static_memory_resource.h"
#include <type_traits>

using namespace stdsharp;
using namespace std;

using allocator_t = allocator<generic_storage>;

SCENARIO("object allocation default initializable", "[memory][object allocation]") // NOLINT
{
    // STATIC_REQUIRE(default_initializable<object_allocation_like<int, allocator_t>>);
    // STATIC_REQUIRE(default_initializable<trivial_object_allocation<allocator_t>>);
    // STATIC_REQUIRE(default_initializable<normal_object_allocation<allocator_t>>);
    // STATIC_REQUIRE(default_initializable<
    //                normal_movable_object_allocation<allocator<generic_storage>>>);

    // constexpr special_mem_req worst_req = {
    //     expr_req::ill_formed,
    //     expr_req::ill_formed,
    //     expr_req::ill_formed,
    //     expr_req::ill_formed,
    //     expr_req::well_formed,
    //     expr_req::ill_formed //
    // };

    // STATIC_REQUIRE(::std::is_constructible_v<basic_object_allocation<worst_req, allocator_t>>);
    // STATIC_REQUIRE(::std::is_destructible_v<basic_object_allocation<worst_req, allocator_t>>);
}

SCENARIO("object allocation movable", "[memory][object allocation]") // NOLINT
{
    // normal_object_allocation<allocator_t> v0;
    // normal_object_allocation<allocator_t> v1;

    // v1 = v0;
    // STATIC_REQUIRE(move_assignable<normal_object_allocation<allocator_t>>);
}

void foo() { normal_object_allocation<allocator_t> allocation; }

SCENARIO("object allocation emplace", "[memory][object allocation]") // NOLINT
{
    GIVEN("a normal object allocation")
    {
        // normal_object_allocation<allocator_t> allocation;

        // WHEN("emplace an int value")
        // {
        //     auto value = allocation.emplace<int>(1);

        //     THEN("the return value should correct") { REQUIRE(value == 1); }
        // }

        // WHEN("emplace an int vector")
        // {
        //     const auto list = {1, 2};
        //     const auto& value = allocation.emplace(vector<int>{list});

        //     THEN("the return value should correct")
        //     {
        //         REQUIRE_THAT(value, Catch::Matchers::RangeEquals(list));
        //     }
        // }
    }
}