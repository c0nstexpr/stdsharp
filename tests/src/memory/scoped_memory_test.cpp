#include <span>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include "test.h"
#include "stdsharp/memory/scoped_memory.h"
#include "stdsharp/memory/static_allocator.h"

using namespace stdsharp;
using namespace std;

template<typename T>
struct my_alloc : allocator<T>
{
    vector<void*> allocated;
    vector<void*> deallocated;

    T* allocate(const size_t n)
    {
        const auto ptr = allocator<T>::allocate(n);
        const auto void_p = to_void_pointer(ptr);

        INFO(fmt::format("{} = my_alloc::allocate({})\n", void_p, n));
        allocated.emplace_back(void_p);
        return ptr;
    }

    void deallocate(T* p, const size_t n)
    {
        const auto void_p = to_void_pointer(p);

        INFO(fmt::format("my_alloc::deallocate({}, {})\n", void_p, n));
        allocator<T>::deallocate(p, n);
        deallocated.emplace_back(void_p);
    }

    void leak_check() const noexcept
    {
        REQUIRE_THAT(allocated, Catch::Matchers::RangeEquals(deallocated));
    }
};

SCENARIO("make raii memory", "[memory][raii_memory]") // NOLINT
{
    GIVEN("an allocator")
    {
        my_alloc<int> alloc;

        WHEN("allocate a int")
        {
            const auto mem = scope::allocate(alloc, 1);
            const auto ptr = mem.ptr();

            *ptr = 42;

            INFO(fmt::format("address: {}", to_void_pointer(ptr)));

            REQUIRE(*ptr == 42);
        }

        alloc.leak_check();
    }
}

SCENARIO("make raii memory from allocator tuple", "[memory][raii_memory]") // NOLINT
{
    GIVEN("an allocator tuple")
    {
        ::std::tuple<static_allocator<int, sizeof(int) * 4>, my_alloc<int>> alloc;

        WHEN("allocate a int")
        {
            const auto mem = scope::allocate(alloc, 1);
            const auto ptr = mem.ptr();

            *ptr = 42;

            REQUIRE(*ptr == 42);
        }

        constexpr auto count = 5;
        WHEN(fmt::format("allocate {} ints", count))
        {
            {
                const auto mem = scope::allocate(alloc, count);
                const auto ptr = mem.ptr();

                INFO(fmt::format("address: {}", to_void_pointer(ptr)));

                THEN("write values")
                {
                    constexpr auto data = views::iota(0, count);
                    std::ranges::copy(data, ptr);
                    REQUIRE_THAT(data, Catch::Matchers::RangeEquals(span(ptr, count)));
                }
            }

            get<1>(alloc).leak_check();
        }
    }

    GIVEN("an zero size static allocator tuple")
    {
        ::std::tuple<static_allocator<int, 0>> alloc;

        WHEN("allocate a int")
        {
            REQUIRE_THROWS_AS(scope::allocate(alloc, 1), scope::aggregate_bad_alloc<1>);
        }
    }
}