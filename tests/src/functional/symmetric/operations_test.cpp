#include "functional/symmetric/operations_test.h"
#include "functional/symmetric/operations.h"

namespace stdsharp::test::functional::symmetric
{
    boost::ut::suite& operations_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace std;
            using namespace boost::ut;
            using namespace bdd;
            using namespace utility;

            namespace libfunctional = stdsharp::functional;
            using namespace libfunctional;
            using namespace libfunctional::symmetric;

            feature("symmetric operation cpo") = []
            {
                given("given vector<int> instance") = [](const vector<int>& vec)
                {
                    print(fmt::format("{}", vec));

                    // clang-tidy bug below
                    // NOLINTNEXTLINE(performance-unnecessary-value-param)
                    then("use emplace back") = [](const vector<int>& origin)
                    {
                        auto vec = origin;
                        constexpr auto v = 0;
                        const auto& revert = libfunctional::symmetric::operation_fn{}(
                            containers::actions::emplace_back, vec, v);
                        containers::actions::emplace_back(vec, v);
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
