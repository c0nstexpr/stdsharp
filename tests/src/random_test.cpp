#include "random_test.h"
#include "utility/random.h"

namespace blurringshadow::test::utility
{
    boost::ut::suite& random_test() noexcept
    {
        static boost::ut::suite suite{[]() noexcept {}};
        return suite;
    }
}
