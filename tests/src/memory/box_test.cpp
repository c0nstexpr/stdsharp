#include "stdsharp/memory/box.h"
#include "test_worst_type.h"


using allocator_t = allocator<unsigned char>;

SCENARIO("box basic requirements", "[memory][box]") // NOLINT
{
    using normal_t = normal_box<allocator_t>;
    using unique_t = unique_box<allocator_t>;
    using worst_t = box_for<test_worst_type, allocator_t>;

    STATIC_REQUIRE(default_initializable<box_for<int, allocator_t>>);
    STATIC_REQUIRE(default_initializable<trivial_box<allocator_t>>);
    STATIC_REQUIRE(default_initializable<normal_t>);
    STATIC_REQUIRE(default_initializable<unique_t>);
    STATIC_REQUIRE(default_initializable<worst_t>);

    allocation_type_requirement_test<normal_t, unique_t, worst_t>();
}

SCENARIO("box assign value", "[memory][box]") // NOLINT
{
    allocation_functionality_test<normal_box<allocator_t>>();
}

SCENARIO("constexpr box", "[memory][box]") // NOLINT
{
    STATIC_REQUIRE(
        []
        {
            trivial_box<allocator<int>> allocation{};
            auto& value = allocation.emplace(1);
            value = 42;
            return allocation.get<int>();
        }() == 42
    );
}