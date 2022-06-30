#include <range/v3/view/subrange.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "stdsharp/functional/decompose.h"
#include "stdsharp/containers/actions.h"
#include "test.h"

using namespace stdsharp;
using namespace functional;

struct decompose_test_params
{
    vector<int> initial_v_list;
    initializer_list<int> expected_v_list;
};

SCENARIO("get and decompose") // NOLINT
{
    const auto& params =
        GENERATE(decompose_test_params{{1, 2, 1, 1, 3, 3, 3, 4, 5, 4}, {1, 2, 3, 4, 5}});
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

    INFO(fmt::format("current value: {}", v_list));

    unique_op();

    INFO(fmt::format("after first unique operation, values are: {}", v_list));

    rng | to_decompose | std::ranges::sort;

    INFO(fmt::format("after sort, values are: {}", v_list));

    unique_op();

    REQUIRE(std::ranges::equal(params.expected_v_list, v_list));
} // clang-format on