#include "functional/get_test.h"
#include "functional/get.h"
#include "containers/actions.h"
#include "range/v3/view/subrange.hpp"
#include <utility>

// TODO: MSVC ADL bug
// type declarations to prevent compile errors
using rng = const std::invoke_result_t<::ranges::make_subrange_fn, std::vector<int>&>&;
using t0 = stdsharp::functional::get_t<0, rng>;
using t1 = stdsharp::functional::get_t<1, rng>;

namespace stdsharp::test::functional
{
    using namespace std;
    using namespace std::ranges;
    using namespace boost::ut;
    using namespace bdd;
    using namespace stdsharp::functional;

    boost::ut::suite& get_test()
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