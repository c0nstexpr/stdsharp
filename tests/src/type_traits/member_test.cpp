#include "stdsharp/type_traits/member.h"
#include "test.h"

using namespace std;
using namespace fmt;
using namespace stdsharp;

struct my_class
{
    using mem_t = int;
    mem_t m;

    using mem_func_r_t = char;
    using mem_func_args_t = regular_type_sequence<long, double>;

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    mem_func_r_t mem_f(long, double);
};

SCENARIO("member", "[type traits]") // NOLINT
{
    GIVEN( //
        fmt::format(
            "custom class type\n name: {}\nmember type: {}\nmem func type: {}\nreturn: {}\nargs:"
            "{}",
            type_id<my_class>,
            type_id<my_class::mem_t>,
            type_id<decltype(&my_class::mem_f)>,
            type_id<my_class::mem_func_r_t>,
            type_id<my_class::mem_func_args_t>
        )
    )
    {
        THEN("use member traits to get member type, type should be expected")
        {
            using mem_p_t = stdsharp::member_pointer_traits<&my_class::m>;
            using mem_p_func_t = stdsharp::member_pointer_traits<&my_class::mem_f>;
            using mem_func_r = mem_p_func_t::result_t;
            using mem_func_args = mem_p_func_t::args_t;

            STATIC_REQUIRE(std::same_as<mem_p_t::class_t, my_class>);
            STATIC_REQUIRE(std::same_as<mem_p_t::type, my_class::mem_t>);
            STATIC_REQUIRE(std::same_as<mem_func_r, my_class::mem_func_r_t>);
            STATIC_REQUIRE(std::same_as<mem_func_args, my_class::mem_func_args_t>);
        };
    };
}