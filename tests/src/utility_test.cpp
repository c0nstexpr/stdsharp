#include "utility_test.h"
#include "utility/utility.h"

namespace blurringshadow::test::utility
{
    boost::ut::suite& utility_test() noexcept
    {
        static boost::ut::suite suite{[]() noexcept {}};
        return suite;
    }
}
