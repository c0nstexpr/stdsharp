#include <variant>

#include "utility_test.h"
#include "random_test.h"
#include "algorithm_test.h"
#include "pattern_match_test.h"
#include "traits/value_sequence_test.h"

using namespace blurringshadow::test::utility;

int main()
{
    utility_test();
    random_test();
    algorithm_test();
    pattern_match_test();
    traits::value_sequence_test();
}
