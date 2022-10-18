#include "stdsharp/pattern_match.h"
#include "stdsharp/functional/bind.h"
#include "test.h"

SCENARIO("pattern match", "[pattern match]") // NOLINT
{
    enum class my_enum
    {
        one,
        two,
        three
    };

    using namespace functional;

    GIVEN(
        R"(enum class has three values: "one", "two", "three" and three cases matches separate value)"
    )
    {
        THEN("case 1 match one, case 2 match two, case 3 match three")
        {
            constexpr auto match = []() consteval
            {
                my_enum matched{};
                const auto& matched_assign = functional::bind(assign_v, matched);

                pattern_match(
                    my_enum::two,
                    pair{bind_front(equal_to_v, my_enum::one), matched_assign},
                    pair{bind_front(equal_to_v, my_enum::two), matched_assign},
                    pair{bind_front(equal_to_v, my_enum::three), matched_assign}
                );

                return matched;
            }
            ();

            STATIC_REQUIRE(match == my_enum::two);
        }

        AND_THEN("same for constexpr pattern match")
        {
            STATIC_REQUIRE( //
                requires //
                {
                    constexpr_pattern_match::from_constant<my_enum::two>(
                        [](const type_traits::constant<my_enum::two>) {}
                    );
                }
            );
        }
    }
}