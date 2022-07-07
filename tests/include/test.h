#pragma once

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <fmt/core.h>

namespace stdsharp
{
}

using namespace std;
using namespace fmt;

using fmt::format; // NOLINT

using namespace stdsharp;


template<typename T>
constexpr auto type() noexcept
{
    return __func__;
}