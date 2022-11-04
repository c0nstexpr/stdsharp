#pragma once

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <fmt/core.h>

template<typename T>
constexpr ::std::string_view type() noexcept
{
    return __func__; // NOLINT
}

constexpr auto only_true = [](const auto&) { return true; };
constexpr auto only_false = [](const auto&) { return false; };