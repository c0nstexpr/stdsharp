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

    ALLOCATION_TYPE_REQUIRE(normal_t, unique_t, worst_t);
}

SCENARIO("box assign value", "[memory][box]") // NOLINT
{
    GIVEN("an object allocation")
    {
        normal_box<allocator_t> box_v;

        WHEN("emplace a int")
        {
            const auto const_value = 1;
            const auto& res = box_v.emplace(const_value);
            THEN("the return value should correct") { REQUIRE(res == const_value); }
            AND_THEN("type should be expected")
            {
                REQUIRE(box_v.is_type<std::remove_cvref_t<decltype(const_value)>>());
            }
        }

        WHEN("emplace a array")
        {
            const array const_value = {1u, 2u, 3u};
            const auto& res = box_v.emplace(const_value);
            THEN("the return value should correct") { REQUIRE(res == const_value); }
            AND_THEN("type should be expected")
            {
                REQUIRE(box_v.is_type<std::remove_cvref_t<decltype(const_value)>>());
            }
        }

        WHEN("emplace a vector")
        {
            const vector const_value{1u, 2u, 3u};
            const auto& res = box_v.emplace(const_value);
            THEN("the return value should correct")
            {
                REQUIRE_THAT(res, Catch::Matchers::RangeEquals(const_value));
            }
            AND_THEN("type should be expected")
            {
                REQUIRE(box_v.is_type<std::remove_cvref_t<decltype(const_value)>>());
            }
        }

        {
            auto invoked = 0u;

            struct local : reference_wrapper<unsigned>
            {
                local(unsigned& value): reference_wrapper(value) { ++get(); }
            };

            WHEN("assign custom type twice")
            {
                INFO("custom type");

                box_v.emplace<local>(invoked);
                box_v.emplace<local>(invoked);

                THEN("assign operator should be invoked") { REQUIRE(invoked == 2); }

                AND_THEN("destroy allocation and check content")
                {
                    box_v.reset();
                    REQUIRE(!box_v.has_value());
                }
            }
        }
    }
}

SCENARIO("constexpr box", "[memory][box]") // NOLINT
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