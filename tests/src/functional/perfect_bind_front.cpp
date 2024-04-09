#include "stdsharp/functional/perfect_bind_front.h"
#include "stdsharp/type_traits/object.h"
#include "test.h"

using namespace stdsharp;
using namespace std;

SCENARIO("perfect bind front", "[functional]")
{
    const struct : unique_object
    {
        int value;
    } obj{.value = 42};
}