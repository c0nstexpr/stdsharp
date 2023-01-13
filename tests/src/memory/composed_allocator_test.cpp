#include <list>
#include <memory>

#include "test.h"
#include "stdsharp/memory/composed_allocator.h"


using namespace stdsharp;
using namespace std;

void foo()
{
    void* ptr = nullptr;
    int* iptr = static_cast<int*>(ptr);
    void* ptr2 = iptr;


    // static_assert(inter_convertible<int*, void*>);
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
    STATIC_REQUIRE(allocator_req<allocator<int>>);
}
