#include "stdsharp/functional/symmetric_operations.h"
#include "test.h"

using namespace std;
using namespace stdsharp;

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: symmetric assign operation",
    "[functional][symmetric operation]",
    ((int Value), Value),
    1,
    2,
    3
)
{
    GIVEN(format("int value: {}", Value))
    {
        THEN("plus 1 and revert back")
        {
            auto v = Value;

            const auto& revert = cpo::symmetric_operation(plus_assign_v, v, 1);
            plus_assign_v(v, 1);
            REQUIRE((Value + 1) == v);
            revert();

            REQUIRE(Value == v);
        }
    }
}