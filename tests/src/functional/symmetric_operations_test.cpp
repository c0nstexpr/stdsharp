#include "functional/symmetric_operations_test.h"
#include "functional/symmetric_operations.h"
#include "containers/actions.h"

namespace stdsharp::test::functional
{
    boost::ut::suite& symmetric_operations_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace std;
            using namespace boost::ut;
            using namespace bdd;
            using namespace stdsharp::functional;
            using namespace stdsharp::containers;

            feature("symmetric operation cpo") = []
            {
                given("given a int") = [](const int int_v)
                {
                    print(fmt::format("{}", int_v));

                    // clang-tidy bug below
                    // NOLINTNEXTLINE(performance-unnecessary-value-param)
                    then("assign to 0 and revert back") = [](const int origin)
                    {
                        auto v = origin;
                        const auto& revert = symmetric_operation(assign_v, v, 0);
                        assign_v(v, 0);
                        revert();
                        expect(origin == v) << fmt::format("actual value {}", v);
                    } | tuple{int_v};
                } | tuple{1, 2, 3};

                given("given vector<int> instance") = [](const vector<int>& vec)
                {
                    print(fmt::format("{}", vec));

                    // clang-tidy bug below
                    // NOLINTNEXTLINE(performance-unnecessary-value-param)
                    then("use emplace back") = [](const vector<int>& origin)
                    {
                        auto vec = origin;
                        constexpr auto v = 0;
                        const auto& revert = stdsharp::functional::symmetric_operation(
                            actions::emplace_back, vec, v);
                        actions::emplace_back(vec, v);
                        revert();
                        expect(std::ranges::equal(origin, vec))
                            << fmt::format("actual vec content{}", vec);
                    } | tuple{vec};
                } | tuple{vector<int>{0, 1, 2, 3}};
            };
        };

        return suite;
    }
}
