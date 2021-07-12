#include "random_test.h"
#include "utility/random.h"

boost::ut::suite& random_test()
{
    static boost::ut::suite suite{[] {}};
    return suite;
}
