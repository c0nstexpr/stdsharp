#include "stdsharp/functional/symmetric_operations.h"
#include "stdsharp/containers/actions.h"
#include "test.h"

using namespace std;
using namespace stdsharp;
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
            const auto& revert = symmetric_operation(assign_v, v, 0);
            assign_v(v, 0);
            revert();
            REQUIRE(Value == v);
        }
    }
}

SCENARIO("symmetric vector operation", "[functional][symmetric operation]") // NOLINT
{
    auto list = {0, 1, 2, 3};
    GIVEN(fmt::format("int list: {}", list))
    {
        THEN("use emplace back")
        {
            vector vec(list);
            constexpr auto v = 0;
            const auto& revert =
                stdsharp::functional::symmetric_operation(actions::emplace_back, vec, v);
            actions::emplace_back(vec, v);
            revert();

            REQUIRE(std::ranges::equal(list, vec));
        }
    }
}