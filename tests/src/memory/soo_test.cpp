#include "stdsharp/memory/soo.h"
#include "test_worst_type.h"

SCENARIO("soo allocation basic requirements", "[memory][small object optimization]") // NOLINT
{
    using normal_t = normal_soo_box<>;
    using unique_t = unique_soo_box<>;
    using worst_t = soo_box_for<test_worst_type>;

    STATIC_REQUIRE(::std::constructible_from<trivial_soo_box<>, trivial_soo_box<>::allocator_type>);
    STATIC_REQUIRE(::std::constructible_from<normal_t, normal_t::allocator_type>);
    STATIC_REQUIRE(::std::constructible_from<unique_t, normal_t::allocator_type>);
    STATIC_REQUIRE(::std::constructible_from<worst_t, normal_t::allocator_type>);

    allocation_type_requirement_test<normal_t, unique_t, worst_t>();
}

SCENARIO("use soo allocation store value", "[memory][small object optimization]") // NOLINT
{
    single_stack_buffer<> buffer{};
    normal_soo_box<> box{make_soo_allocator(buffer, {})};
    allocation_functionality_test(box);
}