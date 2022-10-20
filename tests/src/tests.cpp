#include <catch2/catch_test_macros.hpp>

#include "stdsharp/functional/invocables.h"
#include "stdsharp/tuple/tuple.h"

using namespace stdsharp;
using namespace std;

struct a
{
    void operator()() {}
};

struct b
{
    void operator()(int) {}
};

constexpr auto func = get<0>(type_traits::indexed_type<a, 0>{});

SCENARIO("1: All test cases reside in other .cpp files (empty)") {} // NOLINT