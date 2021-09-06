#include "traits/member_test.h"
#include "utility/traits/member.h"

namespace stdsharp::test::utility::traits
{
    boost::ut::suite& member_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace boost::ut;
            using namespace bdd;
            using namespace stdsharp::utility;
            using namespace stdsharp::utility::traits;

            feature("member") = []
            {
                struct my_class
                {
                    using mem_t = int;
                    int m;

                    using mem_func_r_t = char;
                    using mem_func_args_t = type_sequence<long, double>;

                    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
                    char mem_f(long, double) { return {}; }
                };

                given("custom class type") = []
                {
                    print( //
                        fmt::format(
                            "name: {}\nmember type: {}\nmem func type: {}\nreturn: {}\nargs: {}",
                            reflection::type_name<my_class>(),
                            reflection::type_name<my_class::mem_t>(),
                            reflection::type_name<decltype(&my_class::mem_f)>(),
                            reflection::type_name<my_class::mem_func_r_t>(),
                            reflection::type_name<my_class::mem_func_args_t>() // clang-format off
                        ) // clang-format on
                    );

                    then(
                        "use member traits to get member type, type should be expected" // clang-format off
                    ) = [] // clang-format on
                    {
                        using mem_p_t = member_pointer_traits<&my_class::m>;
                        using mem_p_func_t = member_function_pointer_traits<&my_class::mem_f>;
                        using mem_func_r = mem_p_func_t::result_t;
                        using mem_func_args = mem_p_func_t::args_t;

                        static_expect<std::same_as<mem_p_t::class_t, my_class>>() << //
                            fmt::format(
                                "owner type is not \"my_class\", actually {}",
                                reflection::type_name<mem_p_t::class_t>() //
                            );
                        static_expect<std::same_as<mem_p_t::type, my_class::mem_t>>() << //
                            fmt::format(
                                "actually member type {}",
                                reflection::type_name<mem_p_t::type>() //
                            );
                        static_expect<std::same_as<mem_func_r, my_class::mem_func_r_t>>() << //
                            fmt::format(
                                "actually member func return type {}",
                                reflection::type_name<mem_func_r>() //
                            );
                        static_expect< //
                            std::same_as<
                                mem_func_args,
                                my_class::mem_func_args_t // clang-format off
                            >
                        >() << fmt::format( 
                            "actually member func args type {}",
                            reflection::type_name<mem_func_args>()
                        ); // clang-format on
                    };
                };
            };
        };

        return suite;
    }
}
