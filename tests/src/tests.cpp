#include "utility/utility_test.h"
#include "random_test.h"
#include "algorithm_test.h"
#include "pattern_match_test.h"
#include "concurrent_object_test.h"
#include "type_traits/value_sequence_test.h"
#include "type_traits/type_sequence_test.h"
#include "type_traits/member_test.h"
#include "property/property_test.h"

#include <fstream>

int main()
{
    using namespace stdsharp::test;

    utility::utility_test();
    random::random_test();
    algorithm::algorithm_test();
    pattern_match_test();
    concurrent_object_test();
    type_traits::value_sequence_test();
    type_traits::type_sequence_test();
    type_traits::member_test();
    property::property_test();
}
