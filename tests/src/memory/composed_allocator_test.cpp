#include <list>

#include "test.h"
#include "stdsharp/memory/composed_allocator.h"


using namespace stdsharp;
using namespace std;

void foo()
{
    list<int> l(allocator<int>{});

    l.emplace_back(1);
}

template<typename T>
struct my_alloc : allocator<T>
{
    constexpr T* allocate(const size_t n)
    {
        INFO("my_alloc::allocate(" << n << ")");
        return allocator<T>::allocate(n);
    }

    constexpr void deallocate(T* p, const size_t n)
    {
        INFO("my_alloc::deallocate(" << p << ", " << n << ")");
        allocator<T>::deallocate(p, n);
    }
};

SCENARIO( // NOLINT
    "check static allocator for allocator named requirement",
    "[memory][static_allocator]"
)
{
    STATIC_REQUIRE(allocator_req<composed_allocator<int, allocator<int>, my_alloc<int>>>);
}
