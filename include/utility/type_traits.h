// Created by BlurringShadow at 2021-03-01-下午 9:00

#pragma once

#include "utility_core.h"

namespace blurringshadow::utility
{
    template<typename T, typename U>
    concept not_same_as = !std::same_as<T, U>;

    template<typename T>
    using add_const_lvalue_ref_t = std::add_lvalue_reference_t<std::add_const_t<T>>;

    template<typename T>
    concept const_lvalue_ref = std::is_lvalue_reference_v<T> && std::is_const_v<T>;

    template<typename T, typename U>
    concept convertible_from = std::convertible_to<U, T>;

    template<typename T, typename U>
    concept inter_convertible = std::convertible_to<T, U> && convertible_from<T, U>;

    template<std::size_t I>
    using index_constant = std::integral_constant<std::size_t, I>;

    template<auto Value>
    struct constant
    {
        using value_type = decltype(Value);

        static constexpr auto value = Value;

        constexpr operator const value_type&() const noexcept { return value; }

        [[nodiscard]] constexpr auto& operator()() const noexcept { return value; }
    };

    template<auto Value>
    inline constexpr auto constant_v = constant<Value>::value;

    template<auto Value>
    inline constexpr auto constant_t = constant<Value>::value_type;

    template<typename Func, typename... Args>
    concept nothrow_invocable = std::is_nothrow_invocable_v<Func, Args...>;

    // clang-format off
    template<typename Func, typename... Args>
    concept invocable_and_return_any = std::invocable<Func, Args...> && 
        not_same_as<std::invoke_result_t<Func, Args...>, void>;
    // clang-format on

    // clang-format off
    template<typename Func, typename... Args>
    concept nothrow_invocable_and_return_any = nothrow_invocable<Func, Args...> && 
        not_same_as<std::invoke_result_t<Func, Args...>, void>;
    // clang-format on
}
