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

        static constexpr bool try_lock() { return true; }

        static constexpr void unlock_shared() {}

        my_mutex() = default;
        my_mutex(const my_mutex&) = delete;
        my_mutex(my_mutex&&) = delete;
        my_mutex& operator=(const my_mutex&) = delete;
        my_mutex& operator=(my_mutex&&) = delete;
        ~my_mutex() = default;
    };

    STATIC_REQUIRE(!movable<synchronizer<int, my_mutex>>);
    STATIC_REQUIRE(default_initializable<synchronizer<int, my_mutex>>);
    STATIC_REQUIRE(constructible_from<synchronizer<int, my_mutex>, int>);
}

SCENARIO("synchronizer reflection support", "[synchronizer]") // NOLINT
{
    using namespace reflection;
    using namespace stdsharp::literals;

    using synchronizer = synchronizer<int>;

    synchronizer syn{};

    syn.read([](const int& value) { REQUIRE(value == 0); });
    syn.write([](int& value) { value = 43; });

    constexpr auto read_fn = reflection::member_of<synchronizer, "read"_ltr>();
    constexpr auto write_fn = reflection::member_of<synchronizer, "write"_ltr>();

    read_fn(syn, [](const int& value) { REQUIRE(value == 43); });
    write_fn(syn, [](int& value) { value = 56; });
    read_fn(syn, [](const int& value) { REQUIRE(value == 56); });
}