#include "stdsharp/default_operator.h"
#include "stdsharp/memory/pointer_traits.h"
#include "test.h"

SCENARIO("pointer traits", "[memory]") // NOLINT
{
    GIVEN("a void fancy pointer")
    {
        struct fancy_pointer
        {
            using element_type = void;
        };

        using traits = stdsharp::pointer_traits<fancy_pointer>;

        STATIC_REQUIRE(std::same_as<traits::pointer, fancy_pointer>);
        STATIC_REQUIRE(std::same_as<traits::difference_type, ptrdiff_t>);
        STATIC_REQUIRE(std::same_as<traits::element_type, void>);
        STATIC_REQUIRE( //
            !requires {
                requires( //
                    []<typename T = traits> { return requires { typename T::raw_pointer; }; }()
                );
            }
        );
    }

    GIVEN("a int fancy pointer without element type")
    {
        struct fancy_pointer
        {
            const int& operator*() const
            {
                static constexpr int v{};

                return v;
            }
        };

        using traits = stdsharp::pointer_traits<fancy_pointer>;

        static_assert(dereferenceable<fancy_pointer>);

        STATIC_REQUIRE(std::same_as<traits::pointer, fancy_pointer>);
        STATIC_REQUIRE(std::same_as<traits::difference_type, ptrdiff_t>);
        STATIC_REQUIRE(std::same_as<traits::element_type, const int>);
        STATIC_REQUIRE(std::same_as<traits::raw_pointer, const int*>);
    }

    GIVEN("a int fancy pointer")
    {
        struct fancy_pointer
        {
            const int* ptr;

            [[nodiscard]] const int& operator*() const noexcept { return *ptr; }
        };

        using traits = stdsharp::pointer_traits<fancy_pointer>;

        STATIC_REQUIRE(std::same_as<traits::pointer, fancy_pointer>);
        STATIC_REQUIRE(std::same_as<traits::difference_type, ptrdiff_t>);
        STATIC_REQUIRE(std::same_as<traits::element_type, const int>);
        STATIC_REQUIRE(std::same_as<traits::raw_pointer, const int*>);

        int value = 42;
        int* ptr = &value;
        fancy_pointer fancy{ptr};

        WHEN("pointer to") { REQUIRE(*traits::pointer_to(value) == 42); }
        WHEN("to pointer") { REQUIRE(*traits::to_pointer(ptr) == 42); }
        WHEN("to address") { REQUIRE(*traits::to_address(fancy) == 42); }
        WHEN("to void pointer") { REQUIRE(to_void_pointer(ptr) == to_void_pointer(fancy)); }
    }

    GIVEN("a int pointer")
    {
        using traits = stdsharp::pointer_traits<int*>;

        STATIC_REQUIRE(std::same_as<traits::pointer, int*>);
        STATIC_REQUIRE(std::same_as<traits::difference_type, ptrdiff_t>);
        STATIC_REQUIRE(std::same_as<traits::element_type, int>);
        STATIC_REQUIRE(std::same_as<traits::raw_pointer, int*>);

        int value = 42;
        int* ptr = &value;

        WHEN("pointer to") { REQUIRE(*traits::pointer_to(value) == 42); }
        WHEN("to pointer") { REQUIRE(*traits::to_pointer(ptr) == 42); }
        WHEN("to address") { REQUIRE(*traits::to_address(ptr) == 42); }
        WHEN("to void pointer") { REQUIRE(to_void_pointer(ptr) == to_void_pointer(&value)); }
    }
}