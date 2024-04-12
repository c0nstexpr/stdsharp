#include "stdsharp/memory/box.h"
#include "stdsharp/type_traits/expression.h"
#include "test_worst_type.h"

using allocator_t = allocator<unsigned char>;

SCENARIO("box basic requirements", "[memory]") // NOLINT
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

SCENARIO("box assign value", "[memory]") // NOLINT
{
    allocation_functionality_test<normal_box<allocator_t>>();
}

void foo()
{
    int v0{};
    launder_iterator v1{&v0};

    auto v2 = v1.operator++()(1);
    auto v3 = ++v1;

    static_assert(random_access_iterator<decltype(v1)>);

    auto v = std::views::counted(v1, size_t{0});
}

SCENARIO("constexpr box", "[memory]") // NOLINT
{
    STATIC_REQUIRE(
        []
        {
            trivial_box<allocator<int>> b{};
            auto& value = b.emplace(1);
            value = 42;
            return b.get<int>();
        }() == 42
    );
}