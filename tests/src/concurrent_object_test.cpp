#include "concurrent_object_test.h"
#include "concurrent_object.h"

namespace stdsharp::test::type_traits
{
    boost::ut::suite& concurrent_object_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace std;
            using namespace boost::ut;
            using namespace bdd;

            feature("construct") = []
            {
                static_expect<default_initializable<concurrent_object<int>>>();
            };
        };

        return suite;
    }
}