#include "test.h"
#include "stdsharp/concurrent_object.h"

using namespace std;
using namespace fmt;
using namespace stdsharp;

SCENARIO("concurrent object", "[concurrent object]") // NOLINT
{
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

    using function = reflection::function_t<concurrent_object_t>;

    using func = function::get_t<"read"_ltr>();
    concurrent_object_t obj;
    void (*ptr)(const ::std::optional<int>) = nullptr;

    func{}(obj, *ptr);

    STATIC_REQUIRE( //
        invocable<
            function::get_t<"read"_ltr>(),
            concurrent_object_t,
            void(const ::std::optional<int>) // clang-format off
        > // clang-format on
    );
}
