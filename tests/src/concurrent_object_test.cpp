#include "concurrent_object_test.h"
#include "concurrent_object.h"

namespace stdsharp::test
{
    boost::ut::suite& concurrent_object_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace std;
            using namespace boost::ut;
            using namespace bdd;
            using namespace concepts;

            feature("construct") = []
            {
                struct my_mutex
                {
                    void lock() {}
                    void unlock() {}
                    void lock_shared() {}
                    bool try_lock_shared() // NOLINT(readability-convert-member-functions-to-static)
                    {
                        return true;
                    }
                    void unlock_shared() {}
                };

                static_expect<default_initializable<concurrent_object<int>>>();
                static_expect<copyable<concurrent_object<int>>>();
                static_expect<movable<concurrent_object<int>>>();
                static_expect<
                    constructible_from<concurrent_object<int>, concurrent_object<int, my_mutex>>>();
                static_expect< //
                    constructible_from<
                        concurrent_object<int>,
                        const concurrent_object<int, my_mutex>& // clang-format off
                    >
                >(); // clang-format on

                static_expect<
                    assignable<concurrent_object<int>&, concurrent_object<int, my_mutex>>>();
                static_expect< //
                    assignable<
                        concurrent_object<int>&,
                        const concurrent_object<int, my_mutex>& // clang-format off
                    >
                >(); // clang-format on
            };
        };

        return suite;
    }
}
