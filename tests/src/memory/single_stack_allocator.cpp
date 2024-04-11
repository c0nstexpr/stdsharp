#include "test.h"
#include "stdsharp/memory/single_stack_allocator.h"

SCENARIO("single stack allocator", "[memory][single_stack_allocator]") // NOLINT
{
    struct base
    {
        base() = default;
        base(const base&) = default;
        base(base&&) = default;
        base& operator=(const base&) = default;
        base& operator=(base&&) = default;
        virtual ~base() = default;
        [[nodiscard]] constexpr virtual int foo() const = 0;
    };

    struct derived : base
    {
        int v = 0;

        constexpr derived(const int v): v(v) {}

        [[nodiscard]] constexpr int foo() const override { return v; }
    };

    single_stack_buffer<sizeof(derived)> rsc;

    GIVEN("allocator with " << decltype(rsc)::size << " bytes")
    {
        auto allocator = make_single_stack_allocator<derived>(rsc);

        THEN("constructing a derived type at allocator")
        {
            const auto p1 = allocator.allocate(1);

            const base* const ptr = p1;
            constexpr int v = 42;

            std::ranges::construct_at(p1, v);

            REQUIRE(ptr->foo() == v);

            allocator.deallocate(p1, 1);
        }

        THEN("constructing a derived type at allocator")
        {
            const auto p1 = allocator.allocate(1);

            const derived* const ptr = p1;
            constexpr int v = 42;

            std::ranges::construct_at(p1, v);

            REQUIRE(ptr->foo() == v);

            allocator.deallocate(p1, 1);
        }

        THEN("allocate memory more then its size should throws")
        {
            REQUIRE_THROWS_AS(allocator.allocate(decltype(allocator)::size + 1), bad_alloc);
        }
    }

#if __cpp_constexpr >= 202306L
    GIVEN("allocator with 4 * sizeof(int), constexpr allocate and deallocate")
    {
        STATIC_REQUIRE(
            []
            {
                single_stack_buffer<4 * sizeof(int)> rsc;
                auto allocator = make_single_stack_allocator<int>(rsc);
                const auto p1 = allocator.allocate(1);
                const bool is_null = p1 == nullptr;

                allocator.deallocate(p1, 1);

                return !is_null;
            }()
        );
    }
#endif
}
