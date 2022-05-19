#include <range/v3/view/subrange.hpp>

#include "functional/decompose_test.h"
#include "stdsharp/functional/decompose.h"
#include "stdsharp/containers/actions.h"
namespace stdsharp::test::functional
{
    using namespace std;
    using namespace std::ranges;
    using namespace boost::ut;
    using namespace bdd;
    using namespace stdsharp::functional;

    boost::ut::suite& decompose_test()
    {
        static boost::ut::suite suite = []
        {
            struct decompose_test_params
            {
                vector<int> initial_v_list;
                initializer_list<int> expected_v_list;
            }; // clang-format off

            feature("get and decompose") = [](decompose_test_params params) // clang-format on
            {
                auto& v_list = params.initial_v_list;
                const auto& rng = ::ranges::make_subrange(v_list);

                const auto unique_op = [&v_list, &rng]
                {
                    actions::erase(
                        v_list,
                        rng | to_decompose | ::ranges::unique,
                        v_list.cend() //
                    );
                };

                println(fmt::format("current value: {}", v_list));

                unique_op();

                println(fmt::format("after first unique operation, values are: {}", v_list));

                rng | to_decompose | std::ranges::sort;

                println(fmt::format("after sort, values are: {}", v_list));

                unique_op();

                expect(std::ranges::equal(params.expected_v_list, v_list)) << //
                    fmt::format("actual values are: {}", v_list); // clang-format off
            } | tuple{
                decompose_test_params{{1, 2, 1, 1, 3, 3, 3, 4, 5, 4}, {1, 2, 3, 4, 5}}
            }; // clang-format on
        };

        return suite;
    }
}