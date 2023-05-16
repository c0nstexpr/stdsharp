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
    using unique_t = unique_soo_allocation<>;
    using worst_t = soo_allocation_for<local>;

    STATIC_REQUIRE(::std::constructible_from<
                   trivial_soo_allocation<>,
                   trivial_soo_allocation<>::allocator_type>);
    STATIC_REQUIRE(::std::constructible_from<normal_t, normal_t::allocator_type>);
    STATIC_REQUIRE(::std::constructible_from<unique_t, normal_t::allocator_type>);
    STATIC_REQUIRE(::std::constructible_from<worst_t, normal_t::allocator_type>);

    STATIC_REQUIRE(nothrow_movable<unique_t>);
    STATIC_REQUIRE(nothrow_swappable<unique_t>);
    STATIC_REQUIRE(copyable<normal_t>);

    STATIC_REQUIRE(nothrow_movable<worst_t>);
    STATIC_REQUIRE(nothrow_swappable<worst_t>);
}

SCENARIO("use soo allocation store value", "[memory][small object optimization]") // NOLINT
{
    GIVEN("a normal object allocation")
    {
        normal_soo_allocation<> allocation;

        WHEN("emplace an int value")
        {
            auto value = allocation.emplace<int>(1);

            THEN("the return value should correct") { REQUIRE(value == 1); }

            AND_THEN("type should be expected") { REQUIRE(allocation.type() == type_id<int>); }
        }
    }
}