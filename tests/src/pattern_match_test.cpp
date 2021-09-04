#include "pattern_match_test.h"
#include "utility/utility.h"
#include "utility/pattern_match.h"

namespace blurringshadow::test::utility
{
    enum class my_enum
    {
        one,
        two,
        three
    };

    boost::ut::suite& pattern_match_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace std;
            using namespace boost::ut;
            using namespace bdd;
            using namespace blurringshadow::utility;

            feature("pattern_match") = []
            {
                given(R"(given enum class has three values: "one", "two", "three")") = []
                {
                    given("given three cases matches separate value") = []
                    {
                        print("case 1 match one, case 2 match two, case 3 match three");

                        when("when case 2 match set the flag to true") = []
                        {
                            constexpr auto pair_v = []
                            {
                                auto flag = false;
                                my_enum matched{};
                                const auto& matched_assign = bind_ref_front(assign_v, matched);

                                pattern_match(
                                    my_enum::two,
                                    pair{
                                        bind_front(equal_to_v, my_enum::one),
                                        matched_assign,
                                    },
                                    pair{
                                        bind_front(equal_to_v, my_enum::two),
                                        [&flag, &matched_assign](const my_enum e) noexcept
                                        {
                                            matched_assign(e);
                                            flag = true; //
                                        } //
                                    },
                                    pair{
                                        bind_front(equal_to_v, my_enum::three),
                                        matched_assign //
                                    } //
                                );

                                return pair{flag, matched};
                            }();

                            static_expect<pair_v.first>()
                                << fmt::format("actually match {}", to_underlying(pair_v.second));
                        };
                    };
                };
            };

            feature("constexpr_pattern_match") = []
            {
                given(R"(given enum class has three values: "one", "two", "three")") = []
                {
                    given("given three cases matches separate value") = []
                    {
                        print("case 1 match one, case 2 match two, case 3 match three");

                        when("when case 2 match set the flag to true") = []
                        {
                            constexpr auto pair_v = []() noexcept
                            {
                                auto flag = false;
                                my_enum matched{};

                                constexpr_pattern_match::from_constant<my_enum::two>(
                                    [&matched](const type_identity<constant<my_enum::one>>) noexcept
                                    {
                                        matched = my_enum::one; //
                                    }, // clang-format off
                                    [&matched, &flag](const type_identity<constant<my_enum::two>>) noexcept
                                    { // clang-format on
                                        flag = true;
                                        matched = my_enum::two; //
                                    },
                                    [&matched]<my_enum E>( // clang-format off
                                        const type_identity<constant<my_enum::three>>
                                    ) noexcept // clang-format on
                                    {
                                        matched = my_enum::three; //
                                    } //
                                );

                                return pair{flag, matched};
                            }();

                            static_expect<pair_v.first>() << //
                                fmt::format("actually match {}", to_underlying(pair_v.second));
                        };
                    };
                };
            };
        };
        return suite;
    }
}
