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
                    << fmt::format("space size should be plus assignable from int");

                static_expect<::std::invocable<::std::plus<>, int, t>>()
                    << fmt::format("int should be plus assignable from space size");

                static_expect < requires(t v)
                {
                    -v;
                    ~v;
                }
                > () << fmt::format("int should be plus assignable from space size");
            };
        };
        return suite;
    }
}
