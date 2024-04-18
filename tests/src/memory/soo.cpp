#include "stdsharp/memory/soo.h"
#include "test_worst_type.h"

SCENARIO("soo allocation basic requirements", "[memory][small object optimization]") // NOLINT
{
    using normal_t = normal_soo_box<>;
    using unique_t = unique_soo_box<>;
    using worst_t = soo_box_for<test_worst_type>;

    STATIC_REQUIRE(constructible_from<trivial_soo_box<>, trivial_soo_box<>::allocator_type>);
    STATIC_REQUIRE(constructible_from<normal_t, normal_t::allocator_type>);
    STATIC_REQUIRE(constructible_from<unique_t, normal_t::allocator_type>);
    STATIC_REQUIRE(constructible_from<worst_t, normal_t::allocator_type>);

    ALLOCATION_TYPE_REQUIRE(normal_t, unique_t, worst_t);
}

SCENARIO("use soo allocation store value", "[memory][small object optimization]") // NOLINT
{
    GIVEN("an object allocation")
    {
        single_stack_buffer<> buffer{};
        normal_soo_box<> box_v{make_soo_allocator(buffer, {})};

        WHEN("emplace a vector")
        {
            const vector const_value {1, 2, 3};
            const auto& res = box_v.emplace(const_value);
            THEN("the return value should correct") { REQUIRE(res == const_value); }
            AND_THEN("type should be expected")
            {
                REQUIRE(box_v.is_type<std::remove_cvref_t<decltype(const_value)>>());
            }
        }
    }
}