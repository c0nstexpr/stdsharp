#pragma once

#include "stdsharp/concepts/concepts.h"
#include "test.h"

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

#define ALLOCATION_TYPE_REQUIRE(Normal, Unique, Worst) \
    STATIC_REQUIRE(copyable<Normal>);                  \
    STATIC_REQUIRE(nothrow_movable<Unique>);           \
    STATIC_REQUIRE(nothrow_swappable<Unique>);         \
    STATIC_REQUIRE(nothrow_movable<Worst>);            \
    STATIC_REQUIRE(nothrow_swappable<Worst>)