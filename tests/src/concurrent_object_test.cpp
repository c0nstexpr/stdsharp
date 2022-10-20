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

    STATIC_REQUIRE(copyable<concurrent_object<int>>);
    STATIC_REQUIRE(movable<concurrent_object<int>>);

    STATIC_REQUIRE(default_initializable<concurrent_object<int>>);
    STATIC_REQUIRE(constructible_from<concurrent_object<int>, concurrent_object<int, my_mutex>>);
    STATIC_REQUIRE(constructible_from<
                   concurrent_object<int>,
                   const concurrent_object<int, my_mutex>&>);

    STATIC_REQUIRE(assignable<concurrent_object<int>&, concurrent_object<int, my_mutex>>);
    STATIC_REQUIRE(assignable<concurrent_object<int>&, const concurrent_object<int, my_mutex>&>);
}

SCENARIO("concurrent object reflection support", "[concurrent object]") // NOLINT
{
    using namespace reflection;
    using namespace stdsharp::literals;

    using concurrent_object_t = concurrent_object<int>;

    constexpr auto members = get_members<concurrent_object_t>();

    STATIC_REQUIRE( //
        std::ranges::equal(
            members,
            initializer_list<member_info>{
                {"read", member_category::function},
                {"write", member_category::function} //
            }
        )
    );

    STATIC_REQUIRE( //
        invocable<
            decltype(get_member<"read"_ltr>(concurrent_object_t{})),
            void(const ::std::optional<int>)
        >
    );
}
