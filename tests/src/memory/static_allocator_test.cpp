#include "stdsharp/memory/static_memory_resource.h"
#include "test.h"
#include "stdsharp/memory/static_allocator.h"
#include "stdsharp/memory/allocator_traits.h"
#include "stdsharp/memory/pointer_traits.h"

using namespace stdsharp;
using namespace std;

// SCENARIO("static allocator", "[memory][static_allocator]") // NOLINT
// {
//     struct base // NOLINT(*-special-member-functions)
//     {
//         virtual ~base() = default;
//         [[nodiscard]] constexpr virtual int foo() const = 0;
//     };

//     struct derived : base
//     {
//         int v = 0;

//         constexpr derived(const int v): v(v) {}

//         [[nodiscard]] constexpr int foo() const override { return v; }
//     };

//     // NOLINTBEGIN(*-reinterpret-cast)

//     static_memory_resource_for<derived, 4> rsc;

//     GIVEN("static allocator with " << decltype(rsc)::size << " bytes")
//     {
//         static_allocator_for<derived, 4> allocator{rsc};

//         THEN("constructing a derived type at allocator")
//         {
//             const auto p1 = allocator.allocate(1);

//             {
//                 const base* base_ptr = p1;
//                 constexpr int v = 42;

//                 std::ranges::construct_at(p1, v);

//                 REQUIRE(base_ptr->foo() == v);
//             }

//             AND_THEN("constructing anther one at allocator")
//             {
//                 const auto p2 = allocator.allocate(1);

//                 {
//                     constexpr int v2 = 42;
//                     const base* base_ptr = p2;

//                     std::ranges::construct_at(p2, v2);

//                     REQUIRE(base_ptr->foo() == v2);
//                 }

//                 allocator.deallocate(p2, 1);
//             }

//             allocator.deallocate(p1, 1);
//         }

//         THEN("constructing two derived types at allocator")
//         {
//             constexpr array<derived, 2> derived_array{42, 13};

//             array<derived*, 2> ptrs{};

//             for(auto it = ptrs.begin(); const auto& d : derived_array)
//             {
//                 auto& d_ptr = *it;

//                 d_ptr = allocator.allocate(1);

//                 std::ranges::construct_at(d_ptr, d);

//                 const base* base_ptr = d_ptr;

//                 REQUIRE(base_ptr->foo() == d.foo());

//                 ++it;
//             }

//             AND_THEN("release first type and construct the third type")
//             {
//                 allocator.deallocate(ptrs.front(), 1);

//                 constexpr derived d3{65};

//                 const auto ptr = allocator.allocate(1);
//                 const base* base3_ptr = ptr;

//                 std::ranges::construct_at(ptr, d3);

//                 REQUIRE(
//                     to_void_pointer(ptr) ==
//                     to_void_pointer(allocator.resource().storage().data())
//                 );
//                 REQUIRE(base3_ptr->foo() == d3.foo());
//                 REQUIRE(ptrs[1]->foo() == derived_array[1].foo());

//                 allocator.deallocate(ptr, 1);
//             }
//         }

//         THEN("allocate memory more then its size should throws")
//         {
//             REQUIRE_THROWS_AS(
//                 allocator.allocate(allocator.size * sizeof(generic_storage)),
//                 bad_alloc
//             );
//         }
//     }

//     GIVEN("allocator with 4 * sizeof(int), constexpr allocate and deallocate")
//     {
//         STATIC_REQUIRE(
//             []
//             {
//                 static_memory_resource<2> rsc;
//                 static_allocator allocator{rsc};
//                 const auto p1 = allocator.allocate(1);
//                 const bool is_null = p1 == nullptr;

//                 allocator.deallocate(p1, 1);

//                 return !is_null;
//             }()
//         );
//     }

//     // NOLINTEND(*-reinterpret-cast)
// }
