#include "stdsharp/memory/object_allocation.h"
#include "test_worst_type.h"


using allocator_t = allocator<unsigned char>;

SCENARIO("object allocation basic requirements", "[memory][object allocation]") // NOLINT
{
    using normal_t = normal_obj_allocation<allocator_t>;
    using unique_t = unique_obj_allocation<allocator_t>;
    using worst_t = obj_allocation_for<test_worst_type, allocator_t>;

    STATIC_REQUIRE(default_initializable<obj_allocation_for<int, allocator_t>>);
    STATIC_REQUIRE(default_initializable<trivial_obj_allocation<allocator_t>>);
    STATIC_REQUIRE(default_initializable<normal_t>);
    STATIC_REQUIRE(default_initializable<unique_t>);
    STATIC_REQUIRE(default_initializable<worst_t>);

    allocation_type_requirement_test<normal_t, unique_t, worst_t>();
}

SCENARIO("object allocation assign value", "[memory][object allocation]") // NOLINT
{
    allocation_functionality_test<normal_obj_allocation<allocator_t>>();
}

SCENARIO("constexpr object allocation", "[memory][object allocation]") // NOLINT
{
    STATIC_REQUIRE(
        []
        {
            trivial_obj_allocation<allocator<int>> allocation{};
            auto& value = allocation.emplace(1);
            value = 42;
            return allocation.get<int>();
        }() == 42
    );
}