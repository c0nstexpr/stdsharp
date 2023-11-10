#pragma once

#include "stdsharp/concepts/concepts.h"
#include "test.h"

using namespace stdsharp;
using namespace std;

struct test_worst_type
{
    test_worst_type() = default;
    ~test_worst_type() = default;

private:
    test_worst_type(const test_worst_type&) = default;
    test_worst_type(test_worst_type&&) = default;
    test_worst_type& operator=(const test_worst_type&) = default;
    test_worst_type& operator=(test_worst_type&&) = default;
};

template<typename Normal, typename Unique, typename Worst>
void allocation_type_requirement_test()
{
    STATIC_REQUIRE(copyable<Normal>);
    STATIC_REQUIRE(nothrow_movable<Unique>);
    STATIC_REQUIRE(nothrow_swappable<Unique>);
    STATIC_REQUIRE(nothrow_movable<Worst>);
    STATIC_REQUIRE(nothrow_swappable<Worst>);
}

template<typename T, typename Value, typename Predicate>
void allocation_emplace_value_test(T& box, const Value& value, Predicate predicate)
{
    WHEN(format("emplace a value, type id is {}", type_id<Value>))
    {
        const auto& res = box.emplace(value);

        THEN("the return value should correct") { predicate(res, value); }

        AND_THEN("type should be expected") { REQUIRE(box.template is_type<Value>()); }
    }
}

template<typename T>
void allocation_emplace_execution_test(T& box)
{
    auto invoked = 0u;

    struct local : reference_wrapper<unsigned>
    {
        local(unsigned& value): reference_wrapper(value) { ++get(); }
    };

    WHEN("assign custom type twice")
    {
        INFO(format("custom type: {}", type_id<local>));

        box.template emplace<local>(invoked);
        box.template emplace<local>(invoked);

        THEN("assign operator should be invoked") { REQUIRE(invoked == 2); }

        AND_THEN("destroy allocation and check content")
        {
            box.reset();
            REQUIRE(!box.has_value());
        }
    }
}

template<typename T>
void allocation_functionality_test(T box = T{})
{
    GIVEN(format("an object allocation for type id {}", type_id<T>))
    {
        allocation_emplace_value_test(
            box,
            1,
            [](const int v, const int value) { REQUIRE(v == value); }
        );

        allocation_emplace_value_test(
            box,
            vector<int>{1, 2},
            [](const vector<int>& v, const vector<int>& value)
            {
                REQUIRE_THAT(v, Catch::Matchers::RangeEquals(value)); //
            }
        );

        allocation_emplace_execution_test(box);
    }
}