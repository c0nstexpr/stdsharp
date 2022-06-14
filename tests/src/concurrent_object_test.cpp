#include <catch2/catch_test_macros.hpp>

#include "stdsharp/concurrent_object.h"

SCENARIO("concurrent_object", "[concurrent_object]") // NOLINT
{
    using namespace std;
    using namespace stdsharp;
    using namespace stdsharp::concepts;

    struct my_mutex
    {
        void lock() {}
        void unlock() {}
        void lock_shared() {}
        bool try_lock_shared() // NOLINT(*-member-functions-to-static)
        {
            return true;
        }
        void unlock_shared() {}
    };

    REQUIRE(default_initializable<concurrent_object<int>>);
    REQUIRE(copyable<concurrent_object<int>>);
    REQUIRE(movable<concurrent_object<int>>);
    REQUIRE(constructible_from<concurrent_object<int>, concurrent_object<int, my_mutex>>);
    REQUIRE(constructible_from<concurrent_object<int>, const concurrent_object<int, my_mutex>&>);

    REQUIRE(assignable<concurrent_object<int>&, concurrent_object<int, my_mutex>>);
    REQUIRE(assignable<concurrent_object<int>&, const concurrent_object<int, my_mutex>&>);
}