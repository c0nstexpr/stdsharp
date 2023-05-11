#include "test.h"
#include "stdsharp/memory/object_allocation.h"
#include "stdsharp/memory/static_memory_resource.h"

using namespace stdsharp;
using namespace std;

using allocator_t = allocator<generic_storage>;

void foo()
{
    using t = implement_dispatcher<expr_req::no_exception, void, int>;
    using fun_ptr = t::type*;

    constexpr auto fn = [](int&&) noexcept {};
    // t dispathcer{fn};

    static_assert(::std::convertible_to<decltype(fn), t::type*>);

    auto ptr = static_cast<fun_ptr>(fn);

    // using t = normal_object_allocation<allocator_t>;

    // t v0{};
    // t v1{::std::allocator_arg, v0.get_allocator(), cpp_move(v0)};

    // v1.emplace<void>();

    // static_assert(::std::move_constructible<t>);
    // static_assert(nothrow_move_constructible<t>);

    // static_assert(move_assignable<t>);
    // static_assert(nothrow_move_assignable<t>);

    // static_assert(nothrow_movable<t>);
}

SCENARIO("object allocation basic requirements", "[memory][object allocation]") // NOLINT
{
    // STATIC_REQUIRE(default_initializable<object_allocation_like<int, allocator_t>>);
    // STATIC_REQUIRE(default_initializable<trivial_object_allocation<allocator_t>>);
    // STATIC_REQUIRE(default_initializable<normal_object_allocation<allocator_t>>);

    // STATIC_REQUIRE(nothrow_movable<normal_movable_object_allocation<allocator_t>>);
    // STATIC_REQUIRE(nothrow_swappable<normal_movable_object_allocation<allocator_t>>);
    // STATIC_REQUIRE(copyable<normal_object_allocation<allocator_t>>);

    // using worst_allocation = basic_object_allocation<
    //     []
    //     {
    //         auto req = special_mem_req::ill_formed;
    //         req.destruct = expr_req::no_exception;
    //         return req;
    //     }(),
    //     allocator_t // clang-format off
    // >; // clang-format on

    // STATIC_REQUIRE(default_initializable<worst_allocation>);
    // STATIC_REQUIRE(nothrow_movable<worst_allocation>);
    // STATIC_REQUIRE(nothrow_swappable<worst_allocation>);
}

// SCENARIO("object allocation assign value", "[memory][object allocation]") // NOLINT
// {
//     GIVEN("a normal object allocation")
//     {
//         normal_object_allocation<allocator_t> allocation;

//         WHEN("emplace an int value")
//         {
//             auto value = allocation.emplace<int>(1);

//             THEN("the return value should correct") { REQUIRE(value == 1); }
//         }

//         WHEN("emplace an int vector")
//         {
//             const auto list = {1, 2};
//             const auto& value = allocation.emplace(vector<int>{list});

//             THEN("the return value should correct")
//             {
//                 REQUIRE_THAT(value, Catch::Matchers::RangeEquals(list));
//             }
//         }

//         WHEN("assign an int value")
//         {
//             allocation = 1;

//             THEN("the return value should correct") { REQUIRE(allocation.get<int>() == 1); }

//             AND_THEN("type should be expected") { REQUIRE(allocation.type() == type_id<int>); }
//         }

//         struct local
//         {
//             bool invoked = false;

//             local() = default;
//             local(const local&) = default;
//             local(local&&) = default;

//             local& operator=(local&&) noexcept
//             {
//                 invoked = true;
//                 return *this;
//             }

//             local& operator=(const local&) noexcept
//             {
//                 invoked = true;
//                 return *this;
//             }

//             ~local() = default;
//         };

//         WHEN("assign custom type twice")
//         {
//             INFO(fmt::format("custom type: {}", type_id<local>));

//             auto& l = allocation.emplace<local>();
//             allocation.emplace<local>();

//             THEN("assign operator should be invoked") { REQUIRE(l.invoked); }

//             AND_THEN("reset allocation and check content")
//             {
//                 allocation.reset();
//                 REQUIRE(!allocation);
//             }
//         }
//     }
// }

// SCENARIO("constexpr object allocation", "[memory][object allocation]") // NOLINT
// {
//     STATIC_REQUIRE( // TODO: use generic storage type for inner storage
//         []
//         {
//             trivial_object_allocation<allocator<int>> allocation{};
//             auto& value = allocation.emplace(1);
//             value = 42;
//             return allocation.get<int>();
//         }() == 42
//     );
// }
