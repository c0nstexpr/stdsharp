#include "test.h"
#include "stdsharp/memory/small_object_optimization.h"

using namespace stdsharp;
using namespace std;

SCENARIO("soo allocation basic requirements", "[memory][small object optimization]") // NOLINT
{
    struct local
    {
        local() = default;
        ~local() = default;

    private:
        local(const local&) = default;
        local(local&&) = default;
        local& operator=(const local&) = default;
        local& operator=(local&&) = default;
    };

    using normal_t = normal_soo_allocation<>;
    // using unique_t = unique_soo_allocation<>;
    // using worst_t = soo_allocation_for<local>;

    // STATIC_REQUIRE(default_initializable<trivial_obj_allocation<allocator_t>>);
    // STATIC_REQUIRE(default_initializable<normal_t>);
    // STATIC_REQUIRE(default_initializable<unique_t>);

    // STATIC_REQUIRE(nothrow_movable<unique_t>);
    // STATIC_REQUIRE(nothrow_swappable<unique_t>);
    // STATIC_REQUIRE(copyable<normal_t>);

    // STATIC_REQUIRE(default_initializable<worst_t>);
    // STATIC_REQUIRE(nothrow_movable<worst_t>);
    // STATIC_REQUIRE(nothrow_swappable<worst_t>);
}

SCENARIO("soo allocation assign value", "[memory][small object optimization]") // NOLINT
{
    GIVEN("a normal object allocation")
    {
        // normal_obj_allocation<> allocation;

        // WHEN("emplace an int value")
        // {
        //     auto value = allocation.emplace<int>(1);

        //     THEN("the return value should correct") { REQUIRE(value == 1); }

        //     AND_THEN("type should be expected") { REQUIRE(allocation.type() == type_id<int>); }
        // }

        // WHEN("emplace an int vector")
        // {
        //     const auto list = {1, 2};
        //     const auto& value = allocation.emplace(vector<int>{list});

        //     THEN("the return value should correct")
        //     {
        //         REQUIRE_THAT(value, Catch::Matchers::RangeEquals(list));
        //     }

        //     AND_THEN("type should be expected")
        //     {
        //         REQUIRE(allocation.type() == type_id<vector<int>>);
        //     }
        // }

        // auto invoked = 0u;

        // struct local : reference_wrapper<unsigned>
        // {
        //     local(unsigned& value): reference_wrapper(value) { ++get(); }
        // };

        // WHEN("assign custom type twice")
        // {
        //     INFO(fmt::format("custom type: {}", type_id<local>));

        //     auto& l = allocation.emplace<local>(invoked);
        //     allocation.emplace<local>(invoked);

        //     THEN("assign operator should be invoked") { REQUIRE(invoked == 2); }

        //     AND_THEN("destroy allocation and check content")
        //     {
        //         allocation.destroy();
        //         REQUIRE(!allocation);
        //     }
        // }
    }
}
