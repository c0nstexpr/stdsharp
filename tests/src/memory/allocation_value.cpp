#include "stdsharp/cstdint/cstdint.h"
#include "stdsharp/memory/allocation_value.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("vector allocation value", "[memory][allocation_value]")
{
    using allocator_type = std::allocator<stdsharp::byte>;
    using allocation_traits = allocation_traits<allocator_type>;
    using allocator_traits = allocation_traits::allocator_traits;
    using allocation_value_t = allocation_value<allocator_type, vector<int>>;

    allocator_type allocator;
    allocation_value_t allocation_value;
    const auto allocation_1 = allocation_traits::allocate(allocator, sizeof(vector<int>));
    allocation_traits::callocation_result callocation_1 = allocation_1;
    const auto allocation_2 = allocation_traits::allocate(allocator, sizeof(vector<int>));
    const vector<int> seq{1, 2, 3};

    WHEN("default construct")
    {
        allocation_value(allocator, allocation_1, {});

        THEN("value is 0")
        {
            REQUIRE_THAT(
                allocation_value.get(allocation_1),
                Catch::Matchers::RangeEquals(vector<int>{})
            );
        }

        THEN("set the value to int sequence")
        {
            allocation_value.get(allocation_1) = seq;
            REQUIRE_THAT(allocation_value.get(allocation_1), Catch::Matchers::RangeEquals(seq));
        }

        allocation_value(allocator, allocation_1);
    }

    WHEN("constructs and set value to seq")
    {
        allocation_value(allocator, allocation_1, {});
        allocation_value.get(allocation_1) = seq;

        THEN("copy construct")
        {
            allocation_value(allocator, callocation_1, allocation_2);
            THEN("value is seq")
            {
                REQUIRE_THAT(allocation_value.get(allocation_2), Catch::Matchers::RangeEquals(seq));
            }
            allocation_value(allocator, allocation_2);
        }

        THEN("move construct")
        {
            allocation_value(allocator, allocation_1, allocation_2);
            REQUIRE_THAT(allocation_value.get(allocation_2), Catch::Matchers::RangeEquals(seq));
            allocation_value(allocator, allocation_2);
        }

        THEN("copy assign")
        {
            allocation_value(allocator, allocation_2, {});
            allocation_value(callocation_1, allocation_2);
            REQUIRE_THAT(allocation_value.get(allocation_2), Catch::Matchers::RangeEquals(seq));
            allocation_value(allocator, allocation_2);
        }

        THEN("move assign")
        {
            allocation_value(allocator, allocation_2, {});
            allocation_value(allocation_1, allocation_2);
            REQUIRE_THAT(allocation_value.get(allocation_2), Catch::Matchers::RangeEquals(seq));
            allocation_value(allocator, allocation_2);
        }

        allocation_value(allocator, allocation_1);
    }

    allocation_traits::deallocate(allocator, array{allocation_1, allocation_2});
}

SCENARIO("int array allocation value", "[memory][allocation_value]")
{
    using allocator_type = std::allocator<int>;
    using allocation_traits = allocation_traits<allocator_type>;
    using allocator_traits = allocation_traits::allocator_traits;
    using allocation_value_t = allocation_value<allocator_type, int[]>; // NOLINT(*-arrays)

    allocator_type allocator;
    allocation_value_t allocation_value{3};
    const auto allocation_1 =
        allocation_traits::allocate(allocator, allocation_value.size() * sizeof(int));
    allocation_traits::callocation_result callocation_1 = allocation_1;
    const auto allocation_2 =
        allocation_traits::allocate(allocator, allocation_value.size() * sizeof(int));
    const int seq[3]{1, 2, 3}; // NOLINT(*-arrays)

    WHEN("default construct")
    {
        allocation_value(allocator, allocation_1, {});

        THEN("value is 0")
        {
            REQUIRE_THAT(
                allocation_value.get(allocation_1),
                Catch::Matchers::RangeEquals(vector<int>{})
            );
        }

        THEN("set the value to int sequence")
        {
            std::ranges::copy(seq, allocation_value.data(allocation_1));
            REQUIRE_THAT(allocation_value.get(allocation_1), Catch::Matchers::RangeEquals(seq));
        }

        allocation_value(allocator, allocation_1);
    }

    WHEN("constructs and set value to seq")
    {
        allocation_value(allocator, allocation_1, {});
        std::ranges::copy(seq, allocation_value.data(allocation_1));

        THEN("copy construct")
        {
            allocation_value(allocator, callocation_1, allocation_2);
            THEN("value is seq")
            {
                REQUIRE_THAT(allocation_value.get(allocation_2), Catch::Matchers::RangeEquals(seq));
            }
            allocation_value(allocator, allocation_2);
        }

        THEN("move construct")
        {
            allocation_value(allocator, allocation_1, allocation_2);
            REQUIRE_THAT(allocation_value.get(allocation_2), Catch::Matchers::RangeEquals(seq));
            allocation_value(allocator, allocation_2);
        }

        THEN("copy assign")
        {
            allocation_value(allocator, allocation_2, {});
            allocation_value(callocation_1, allocation_2);
            REQUIRE_THAT(allocation_value.get(allocation_2), Catch::Matchers::RangeEquals(seq));
            allocation_value(allocator, allocation_2);
        }

        THEN("move assign")
        {
            allocation_value(allocator, allocation_2, {});
            allocation_value(allocation_1, allocation_2);
            REQUIRE_THAT(allocation_value.get(allocation_2), Catch::Matchers::RangeEquals(seq));
            allocation_value(allocator, allocation_2);
        }

        allocation_value(allocator, allocation_1);
    }

    allocation_traits::deallocate(allocator, array{allocation_1, allocation_2});
}