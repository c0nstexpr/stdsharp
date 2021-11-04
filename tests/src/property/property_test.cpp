#include "property/property_test.h"
#include "property/property.h"

namespace stdsharp::test::property
{
    boost::ut::suite& property_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace std;
            using namespace boost::ut;
            using namespace bdd;
            using namespace stdsharp::functional;
            using namespace stdsharp::property;

            feature("getter and setter") = []
            {
                given("rvalue invoke only func obj") = []
                {
                    struct rvalue_func
                    {
                        constexpr int operator()(const int) && noexcept { return 0; }
                    };

                    static_expect<invocable<setter<rvalue_func>, int>>() << "not invocable";
                };

                given("given integer var 0") = []
                {
                    int i = 0;

                    getter mutable_g{[&i]() mutable { return i; }};
                    const auto& value_g = value_getter(i);
                    const auto& value_s = value_setter(i);
                    setter mutable_s([&i](const int j) mutable { return (i = j); });

                    property_member member{ref(value_g), ref(value_s)};

                    auto value_check = [&value_g, &mutable_g, &member](auto&& pair)
                    {
                        auto&& [setter, expected_v] = pair;

                        setter(expected_v);

                        print(fmt::format("expected value: {}", expected_v));

                        {
                            const auto& r = value_g();
                            expect(r == expected_v) << fmt::format("actual value: {}", r);
                        }
                        {
                            const auto& r = mutable_g();
                            expect(r == expected_v) << fmt::format("actual value: {}", r);
                        }
                        {
                            const auto& r = member.get();
                            expect(r == expected_v) << fmt::format("actual value: {}", r);
                        }
                    };

                    then("invocation result of value getter should match value") = value_check |
                        tuple{
                            pair(ref(value_s), 42),
                            pair(ref(mutable_s), 69),
                            pair{member(reflection::member_t<"set"_ltr>{}), 32} //
                        };
                };
            };
        };

        return suite;
    }
}
