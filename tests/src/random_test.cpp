#include "random_test.h"
#include "utility/random.h"

namespace blurringshadow::test::utility
{
    boost::ut::suite& random_test()
    {
        static boost::ut::suite suite{[] {}};
        return suite;
    }
}
