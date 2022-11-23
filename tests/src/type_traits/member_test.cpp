// #include "stdsharp/type_traits/member.h"
// #include "test.h"

// using namespace std;
// using namespace fmt;
// using namespace stdsharp;

// SCENARIO("member", "[type traits]") // NOLINT
// {
//     struct my_class
//     {
//         using mem_t = int;
//         int m;

//         using mem_func_r_t = char;
//         using mem_func_args_t = stdsharp::type_sequence<long, double>;

//         // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
//         char mem_f(long, double) { return {}; }
//     };

//     GIVEN( //
//         fmt::format(
//             "custom class type\n name: {}\nmember type: {}\nmem func type: {}\nreturn: {}\nargs:"
//             "{}",
//             type<my_class>(),
//             type<my_class::mem_t>(),
//             type<decltype(&my_class::mem_f)>(),
//             type<my_class::mem_func_r_t>(),
//             type<my_class::mem_func_args_t>()
//         )
//     )
//     {
//         THEN("use member traits to get member type, type should be expected")
//         {
//             using mem_p_t = stdsharp::member_pointer_traits<&my_class::m>;
//             using mem_p_func_t = stdsharp::member_function_pointer_traits<&my_class::mem_f>;
//             using mem_func_r = mem_p_func_t::result_t;
//             using mem_func_args = mem_p_func_t::args_t;

//             STATIC_REQUIRE(std::same_as<mem_p_t::class_t, my_class>);
//             STATIC_REQUIRE(std::same_as<mem_p_t::type, my_class::mem_t>);
//             STATIC_REQUIRE(std::same_as<mem_func_r, my_class::mem_func_r_t>);
//             STATIC_REQUIRE(std::same_as<mem_func_args, my_class::mem_func_args_t>);
//         };
//     };
// }