#include <catch2/matchers/catch_matchers_vector.hpp>

#include "test.h"
#include "stdsharp/memory/static_allocator.h"
#include "stdsharp/memory/composed_allocator.h"

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
        REQUIRE_THAT(allocated, Catch::Matchers::Equals(deallocated));
    }

    static constexpr auto to_uintptr_t(const T* const ptr) noexcept
    {
        return bit_cast<uintptr_t>(to_void_pointer(ptr)); // NOLINT
    }
};

SCENARIO( // NOLINT
    "check static allocator for allocator named requirement",
    "[memory][static_allocator]"
)
{
    STATIC_REQUIRE(allocator_req<composed_allocator<int, my_alloc<int>, allocator<int>>>);
}

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: allocate and deallocate",
    "[memory][static_allocator]",
    ((auto Index, typename Alloc), Index, Alloc),
    (0, composed_allocator<int, my_alloc<int>>),
    (1, composed_allocator<int, static_allocator<int, sizeof(int) + sizeof(size_t)>, my_alloc<int>>)
)
{
    GIVEN("a allocator composed by my_alloc")
    {
        Alloc alloc;

        WHEN("allocate a int")
        {
            const auto ptr = alloc.allocate(1);
            *ptr = 42;
            WHEN("deallocate")
            {
                alloc.deallocate(ptr, 1);
                get<Index>(alloc.get_allocators()).leak_check();
            }
        }

        constexpr auto count = 2;
        WHEN(fmt::format("allocate {} ints", count))
        {
            const span<int, count> ptr{alloc.allocate(count), count};

            {
                auto byte_ptr = reinterpret_cast<stdsharp::byte*>(ptr.data()); // NOLINT

                INFO( //
                    fmt::format(
                        "address: {}, forward address: {}, backward address: {}\n",
                        to_void_pointer(ptr.data()),
                        to_void_pointer(byte_ptr + sizeof(size_t)), // NOLINT
                        to_void_pointer(byte_ptr - sizeof(size_t)) // NOLINT
                    )
                );
            }

            THEN("write values")
            {
                std::ranges::copy(views::iota(0, count), ptr.begin());
                WHEN("deallocate")
                {
                    alloc.deallocate(ptr.data(), count);
                    get<Index>(alloc.get_allocators()).leak_check();
                }
            }
        }
    }
}