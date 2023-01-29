#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include "test.h"
#include "stdsharp/memory/raii_memory.h"
#include "stdsharp/memory/static_allocator.h"

using namespace stdsharp;
using namespace std;

template<typename T>
struct my_alloc : allocator<T>
{
    vector<uintptr_t> allocated;
    vector<uintptr_t> deallocated;

    T* allocate(const size_t n)
    {
        const auto ptr = allocator<T>::allocate(n);
        INFO(fmt::format("{} = my_alloc::allocate({})\n", to_uintptr_t(ptr), n));
        allocated.emplace_back(to_uintptr_t(ptr));
        return ptr;
    }

    void deallocate(T* p, const size_t n)
    {
        INFO(fmt::format("my_alloc::deallocate({}, {})\n", to_void_pointer(p), n));
        allocator<T>::deallocate(p, n);
        deallocated.emplace_back(to_uintptr_t(p));
    }

    void leak_check() const noexcept
    {
        REQUIRE_THAT(allocated, Catch::Matchers::RangeEquals(deallocated));
    }

    static auto to_uintptr_t(const T* const ptr) noexcept
    {
        return bit_cast<uintptr_t>(ptr); // NOLINT
    }
};

SCENARIO("make raii memory", "[memory][raii_memory]") // NOLINT
{
    GIVEN("an allocator")
    {
        my_alloc<int> alloc;

        WHEN("allocate a int")
        {
            const auto mem = allocate_raii_memory(alloc, 1);
            const auto ptr = mem.ptr();

            *ptr = 42;

            INFO(fmt::format("address: {}", to_void_pointer(ptr)));

            REQUIRE(mem.ptr() == 42);
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
            const auto mem = allocate_raii_memory(alloc, 1);
            const auto ptr = mem.ptr();

            *ptr = 42;

            REQUIRE(ptr == 42);
        }

        constexpr auto count = 2;
        WHEN(fmt::format("allocate {} ints", count))
        {
            const auto mem = allocate_raii_memory(alloc, 1);
            const auto ptr = mem.ptr();

            INFO(fmt::format("address: {}", to_void_pointer(ptr)));

            THEN("write values")
            {
                constexpr auto data = views::iota(0, count);
                std::ranges::copy(data, ptr);
                REQUIRE_THAT(data, Catch::Matchers::RangeEquals(ptr));
            }
        }
    }
}