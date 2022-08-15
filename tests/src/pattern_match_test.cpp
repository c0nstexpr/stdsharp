#include "stdsharp/pattern_match.h"
#include "stdsharp/functional/operations.h"
#include "stdsharp/functional/functional.h"
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

    using std::type_identity;

    GIVEN(
        R"(enum class has three values: "one", "two", "three" and three cases matches separate value)" //
    )
    {
        THEN("case 1 match one, case 2 match two, case 3 match three")
        {
            constexpr auto match = []
            {
                my_enum matched{};
                const auto& matched_assign = functional::bind(assign_v, matched);

                pattern_match(
                    my_enum::two,
                    pair{
                        bind_front(equal_to_v, my_enum::one),
                        matched_assign,
                    },
                    pair{
                        bind_front(equal_to_v, my_enum::two),
                        matched_assign,
                    },
                    pair{
                        bind_front(equal_to_v, my_enum::three),
                        matched_assign //
                    } //
                );

                return matched;
            }();

            STATIC_REQUIRE(match == my_enum::two);
        }

        AND_THEN("same for constexpr pattern match,")
        {
            constexpr auto match = []() noexcept
            {
                my_enum match{};

                constexpr_pattern_match::from_constant<my_enum::two>(
                    [&match]<my_enum E>(const type_identity<type_traits::constant<E>>) noexcept
                    {
                        match = E; //
                    } //
                );

                return match;
            }();

            STATIC_REQUIRE(match == my_enum::two);
        }
    }
}