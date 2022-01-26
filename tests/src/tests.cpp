#include "utility/utility_test.h"
#include "algorithm_test.h"
#include "pattern_match_test.h"
#include "concurrent_object_test.h"
#include "containers/containers_test.h"
#include "containers/actions_test.h"
#include "type_traits/value_sequence_test.h"
#include "type_traits/type_sequence_test.h"
#include "type_traits/member_test.h"
#include "functional/symmetric/operations_test.h"

#include "random/random.h"
#include "fstream/fstream.h"

int main()
{
    using namespace stdsharp::test;

    utility::utility_test();
    algorithm::algorithm_test();
    pattern_match_test();
    containers::containers_test();
    actions::actions_test();
    concurrent_object_test();
    type_traits::value_sequence_test();
    type_traits::type_sequence_test();
    type_traits::member_test();
    functional::operations_test();
}
