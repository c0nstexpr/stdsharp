#include <fmt/ostream.h>

#include "filesystem/filesystem.h"
#include "filesystem/filesystem_test.h"

namespace stdsharp::test::filesystem
{
    boost::ut::suite& filesystem_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace std;
            using namespace boost::ut;
            using namespace bdd;
            using namespace stdsharp::filesystem;

            feature("space size") = []
            {
                using t = space_size<std::ratio<1>>;

                static_expect<::std::default_initializable<t>>()
                    << fmt::format("space size should be constructbile by default");

                static_expect<::std::invocable<::std::plus<>, t, int>>()
                    << fmt::format("space size should be able to plus int");

                static_expect<::std::invocable<::std::plus<>, int, t>>()
                    << fmt::format("int should be able to plus space size");
                static_expect<::std::invocable<::std::plus<>, t, t>>()
                    << fmt::format("space size should be able plus itself");

                static_expect < requires(t v)
                {
                    +v;
                    -v;
                    ~v;
                }
                > () << fmt::format("space size should have unary operator");

                {
                    constexpr auto v = 1_bit + 1_bit;
                    static_expect<v == 2_bit>()
                        << fmt::format("1 bit + 1 bit should be 2 bit, actually is {}", v);
                }
            };
        };
        return suite;
    }
}
