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

auto foo()
{
    using allocator_type = allocator<int>;
    using box_t = normal_box<allocator<int>>;

    using allocation_traits = allocation_traits<allocator<int>>;
    using allocator_traits = allocation_traits::allocator_traits;
    using allocator_type = allocation_traits::allocator_type;
    using allocation_type = allocation_traits::allocation_result;
    using callocation_type = allocation_traits::callocation_result;
    using allocations_type = std::array<allocation_type, 1>;
    using callocations_type = cast_view<std::ranges::ref_view<allocations_type>, callocation_type>;

    constexpr auto type_req = allocator_traits::type_req<normal_object>;
    constexpr auto vec_req = allocator_traits::type_req<vector<int>>;

    static_assert(type_req.move_construct <= vec_req.move_construct);


    box_t v0{};
    box_t v1{v0};

    v1.emplace<vector<int>, vector<int>>(vector<int>{1, 2});
}