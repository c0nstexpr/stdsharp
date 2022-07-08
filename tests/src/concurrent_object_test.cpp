#include "stdsharp/concurrent_object.h"
#include "test.h"

SCENARIO("concurrent object", "[concurrent object]") // NOLINT
{
    using namespace stdsharp::concepts;

    struct my_mutex
    {
        static constexpr void lock() {}
        static constexpr void unlock() {}
        static constexpr void lock_shared() {}
        static constexpr bool try_lock_shared() { return true; }
        static constexpr void unlock_shared() {}
    };

    REQUIRE(default_initializable<concurrent_object<int>>);
    REQUIRE(copyable<concurrent_object<int>>);
    REQUIRE(movable<concurrent_object<int>>);
    REQUIRE(constructible_from<concurrent_object<int>, concurrent_object<int, my_mutex>>);
    REQUIRE(constructible_from<concurrent_object<int>, const concurrent_object<int, my_mutex>&>);

    REQUIRE(assignable<concurrent_object<int>&, concurrent_object<int, my_mutex>>);
    REQUIRE(assignable<concurrent_object<int>&, const concurrent_object<int, my_mutex>&>);
}