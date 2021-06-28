#include "algorithm_test.h"
#include "utility/algorithm.h"

boost::ut::suite& algorithm_test()
{
    static boost::ut::suite suite = []
    {
        using namespace boost::ut;
        using namespace bdd;
        using namespace blurringshadow::utility;

        feature("set_if") = []<auto First, auto Second>(const static_params<First, Second>)
        {
            given("given two values") = []
            {
                // clang-format off
                auto get_when_test = []<auto CompareTo, auto Func>(
                    const static_params<CompareTo, Func>
                )
                // clang-format on
                {
                    constexpr auto is_set = std::partial_order(Second, First) == CompareTo;
                    constexpr auto then_name = is_set ?
                        "should set first value to second value" :
                        "should not set first value to second value";
                    then(then_name) = [is_set = is_set]
                    {
                        auto first = First;
                        auto second = Second;
                        expect(Func(first, second) == second == is_set);
                    };
                };

                print(fmt::format("first value: {}, second value: {}", First, Second));

                when("set the value if greater") = [&get_when_test] // clang-format off
                {
                    get_when_test(static_params<std::partial_ordering::greater, set_if_greater>{});
                };

                when("set the value if less") = [&get_when_test]
                {
                    get_when_test(static_params<std::partial_ordering::less, set_if_less>{});
                };
            };
            
        } | std::tuple<static_params<1, 2>, static_params<2, 1>>{};
        // clang-format on

        // clang-format off
        feature("is_between") = []<auto Value, auto Min, auto Max>(
          const static_params<Value, Min, Max>
        )
        // clang-format on
        {
            given("given three values") = []
            {
                print(fmt::format("value: {}, min value: {}, max value: {}", Value, Min, Max));
                constexpr auto is_in_range = !(Value < Min) && !(Value > Max);
                then(
                    is_in_range ? // clang-format off
                    "value should between min-max" :
                    "value should not between min-max"
                ) = [is_in_range = is_in_range] { expect(is_between(Value, Min, Max) == is_in_range); };
            };
        } | std::tuple<
            static_params<1, 1, 2>,
            static_params<3, 2, 3>,
            static_params<4, 2, 4>,
            static_params<3, 5, 10>,
            static_params<10, -1, 9>,
            static_params<100u, 50l, 900ll>
        >{};
        // clang-format on
    };

    return suite;
}
