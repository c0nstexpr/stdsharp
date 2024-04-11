#pragma once

#include <vector>

#include "stdsharp/memory/allocator_traits.h"
#include "test.h"

using namespace std;
using namespace stdsharp;

template<typename T>
struct test_allocator : allocator<T>
{
    vector<void*> allocated;
    vector<void*> deallocated;

    T* allocate(const size_t n)
    {
        const auto ptr = allocator<T>::allocate(n);
        const auto void_p = to_void_pointer(ptr);

        INFO(format("{} = my_alloc::allocate({})\n", void_p, n));
        allocated.emplace_back(void_p);
        return ptr;
    }

    void deallocate(T* p, const size_t n) noexcept
    {
        const auto void_p = to_void_pointer(p);

        INFO(format("my_alloc::deallocate({}, {})\n", void_p, n));
        allocator<T>::deallocate(p, n);
        deallocated.emplace_back(void_p);
    }

    void leak_check() const noexcept
    {
        REQUIRE_THAT(allocated, Catch::Matchers::RangeEquals(deallocated));
    }
};

static_assert(allocator_req<test_allocator<int>>);