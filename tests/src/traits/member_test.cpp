#include "traits/member_test.h"
#include "utility/traits/member.h"

namespace blurringshadow::test::utility::traits
{
    boost::ut::suite& member_test() noexcept
    {
        static boost::ut::suite suite = []() noexcept
        {
            using namespace boost::ut;
            using namespace bdd;
            using namespace blurringshadow::utility;

            feature("member") = []() noexcept {};
        };

        return suite;
    }
}
