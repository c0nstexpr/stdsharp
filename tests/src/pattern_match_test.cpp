#include "pattern_match_test.h"
#include "utility/pattern_match.h"

namespace blurringshadow::test::utility
{
    boost::ut::suite& pattern_match_test() noexcept
    {
        static boost::ut::suite suite{[] {}};
        return suite;
    }
}