// Created by BlurringShadow at 2021-03-01-下午 9:00

#pragma once

#include <concepts>
#include <type_traits>

namespace blurringshadow::utility
{
    template<typename T>
    concept enumeration = std::is_enum_v<T>;

    template<typename T, typename U>
    concept not_same_as = !std::same_as<T, U>;

    template<typename T>
    using add_const_lvalue_ref_t = std::add_lvalue_reference_t<std::add_const_t<T>>;

    template<typename T, typename... Args>
    concept nothrow_constructible_from = std::is_nothrow_constructible_v<T, Args...>;

    template<typename T>
    concept nothrow_default_initializable = std::is_nothrow_default_constructible_v<T>;

    template<typename T>
    concept nothrow_move_constructible = std::is_nothrow_move_constructible_v<T>;

    template<typename T>
    concept nothrow_copy_constructible = std::is_nothrow_copy_constructible_v<T>;

    template<typename T, typename U>
    concept nothrow_assignable_from = std::is_nothrow_assignable_v<T, U>;

    template<typename T>
    concept nothrow_copy_assignable = std::is_nothrow_copy_assignable_v<T>;

    template<typename T>
    concept nothrow_move_assignable = std::is_nothrow_move_assignable_v<T>;

    template<typename T>
    concept nothrow_swappable = std::is_nothrow_swappable_v<T>;

    template<typename T, typename U>
    concept nothrow_swappable_with = std::is_nothrow_swappable_with_v<T, U>;

    template<typename T, typename U>
    concept nothrow_convertible_to = std::is_nothrow_convertible_v<T, U>;

    template<typename T, typename U>
    concept convertible_from = std::convertible_to<U, T>;

    template<typename T, typename U>
    concept inter_convertible = std::convertible_to<T, U> && convertible_from<T, U>;

    template<typename T, typename U>
    concept nothrow_convertible_from = std::is_nothrow_convertible_v<U, T>;

    template<typename T, typename U>
    concept nothrow_inter_convertible =
        nothrow_convertible_to<T, U> && nothrow_convertible_from<T, U>;

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

    template<typename Func, typename... Args> // clang-format off
    concept invocable_rnonvoid = std::invocable<Func, Args...> && 
        not_same_as<std::invoke_result_t<Func, Args...>, void>; // clang-format on

    template<typename Func, typename... Args> // clang-format off
    concept nothrow_invocable_rnonvoid = nothrow_invocable<Func, Args...> && 
        not_same_as<std::invoke_result_t<Func, Args...>, void>; // clang-format on

    // c++23 feature
    template<typename Func, typename ReturnT, typename... Args>
    concept invocable_r = std::is_nothrow_invocable_r_v<ReturnT, Func, Args...>;

    template<typename Func, typename ReturnT, typename... Args>
    concept nothrow_invocable_r = std::is_nothrow_invocable_r_v<ReturnT, Func, Args...>;
}
