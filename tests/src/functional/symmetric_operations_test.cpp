#include "stdsharp/functional/symmetric_operations.h"
#include "stdsharp/containers/actions.h"
#include "test.h"

using namespace functional;
using namespace containers;

TEMPLATE_TEST_CASE_SIG( // NOLINT
    "Scenario: symmetric assign operation",
    "[functional][symmetric operation]",
    ((int Value), Value),
    1,
    2,
    3 //
)
{
    GIVEN(fmt::format("int value: {}", Value))
    {
        THEN("assign to 0 and revert back")
        {
            auto v = Value;

            const auto& revert = symmetric_operation(plus_assign_v, v, 1);
            plus_assign_v(v, 1);
            REQUIRE(Value + 1 == v);
            revert();
            REQUIRE(Value == v);
        }
    }
}