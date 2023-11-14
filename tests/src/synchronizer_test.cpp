#include "test.h"
#include "stdsharp/synchronizer.h"

using namespace std;
using namespace stdsharp;

SCENARIO("synchronizer", "[synchronizer]") // NOLINT
{
    struct my_mutex
    {
        static constexpr void lock() {}

        static constexpr void unlock() {}

        static constexpr void lock_shared() {}

        static constexpr bool try_lock_shared() { return true; }

        static constexpr void unlock_shared() {}
    };

    STATIC_REQUIRE(copyable<synchronizer<int>>);
    STATIC_REQUIRE(movable<synchronizer<int>>);

    STATIC_REQUIRE(default_initializable<synchronizer<int>>);
    STATIC_REQUIRE(constructible_from<synchronizer<int>, synchronizer<int, my_mutex>>);
    STATIC_REQUIRE(constructible_from<synchronizer<int>, const synchronizer<int, my_mutex>&>);

    STATIC_REQUIRE(assignable<synchronizer<int>&, synchronizer<int, my_mutex>>);
    STATIC_REQUIRE(assignable<synchronizer<int>&, const synchronizer<int, my_mutex>&>);
}

SCENARIO("synchronizer reflection support", "[synchronizer]") // NOLINT
{
    using namespace reflection;
    using namespace stdsharp::literals;

    using concurrent_object_t = synchronizer<int>;

    using function = reflection::function_t<concurrent_object_t>;

    STATIC_REQUIRE( //
        invocable<
            function::member_of_t<to_array("read")>,
            concurrent_object_t,
            void(const ::std::optional<int>) // clang-format off
        > // clang-format on
    );
}