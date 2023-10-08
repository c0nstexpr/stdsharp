#pragma once

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include "stdsharp/type_traits/core_traits.h"

constexpr auto only_true = [](const auto&) { return true; };
constexpr auto only_false = [](const auto&) { return false; };