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

    template<typename T, std::size_t Size>
    struct array_literal : std::array<T, Size>
    {
        using base = std::array<T, Size>;
        using base::at;
        using base::back;
        using base::base;
        using base::begin;
        using base::cbegin;
        using base::cend;
        using base::crbegin;
        using base::crend;
        using base::empty;
        using base::end;
        using base::fill;
        using base::front;
        using base::rbegin;
        using base::rend;
        using base::size;
        using base::swap;

        constexpr array_literal(const base& input_array): base(input_array) {}

        // clang-format off
        constexpr array_literal(const T (&a)[Size]) { std::copy(a, a + Size, begin()); }
        // clang-format on
    };

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

    template<bool Condition, auto Left, auto Right>
    struct value_conditional : std::conditional_t<Condition, constant<Left>, constant<Right>>
    {
    };

    template<bool Condition, auto Left, auto Right>
    inline constexpr auto value_conditional_v = value_conditional<Condition, Left, Right>::value;

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

    template<auto, std::size_t, typename = increase>
    struct value_next;

    // clang-format off
    template<
        auto From,
        std::size_t Size,
        invocable_and_return_any<decltype(From)> NextFunctor
    >
    // clang-format on
    struct value_next<From, Size, NextFunctor> :
        value_next<functor_invoke<NextFunctor>(From), Size - 1, NextFunctor>
    {
    };

    template<auto From, typename NextFunctor>
    struct value_next<From, 0, NextFunctor> : constant<From>
    {
    };

    // clang-format off
   template<
        auto From,
        std::size_t Size,
        invocable_and_return_any<decltype(From), std::size_t> NextFunctor
    >
    // clang-format on
    struct value_next<From, Size, NextFunctor> : constant<functor_invoke<NextFunctor>(From, Size)>
    {
    };

    template<auto From, std::size_t Size>
    inline constexpr auto value_next_v = value_next<From, Size>::value;
}
