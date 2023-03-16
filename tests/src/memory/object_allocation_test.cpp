#include "test.h"
#include "stdsharp/memory/object_allocation.h"
#include "stdsharp/memory/static_memory_resource.h"

using namespace stdsharp;
using namespace std;

using allocator_t = allocator<generic_storage>;

// SCENARIO("object allocation default initializable", "[memory][object allocation]") // NOLINT
// {
//     STATIC_REQUIRE(default_initializable<object_allocation_like<int, allocator_t>>);
//     STATIC_REQUIRE(default_initializable<trivial_object_allocation<allocator_t>>);
//     STATIC_REQUIRE(default_initializable<normal_object_allocation<allocator_t>>);
//     STATIC_REQUIRE(default_initializable<
//                    normal_movable_object_allocation<allocator<generic_storage>>>);

//     constexpr special_mem_req worst_req = {
//         expr_req::ill_formed,
//         expr_req::ill_formed,
//         expr_req::ill_formed,
//         expr_req::ill_formed,
//         expr_req::well_formed,
//         expr_req::ill_formed //
//     };

//     STATIC_REQUIRE(::std::is_constructible_v<basic_object_allocation<worst_req, allocator_t>>);
// }

// constexpr void foo()
// {
//     const indexed_type<int, 0> indexed{};
//     auto&& v = stdsharp::get_type<0>(indexed);
// }

// SCENARIO("object allocation emplace", "[memory][object allocation]") // NOLINT
// {
//     GIVEN("a trivial object allocation")
//     {
//         trivial_object_allocation<allocator_t> alloc;

//         WHEN("emplace an int value")
//         {
//             // auto value = alloc.emplace<int>(1);

//             // THEN("the return value should correct") { REQUIRE(value == 1); }
//         }
//     }
// }