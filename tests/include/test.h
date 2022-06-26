#pragma once
#include <catch2/catch_template_test_macros.hpp>
#include <fmt/core.h>

using namespace std;
using namespace fmt;

template<typename T>
constexpr auto type() noexcept
{
    return __func__;
}