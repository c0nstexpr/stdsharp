// Created by BlurringShadow at 2021-03-01-下午 9:00

#pragma once

#include <concepts>
#include <type_traits>

#include <range/v3/functional/arithmetic.hpp>

namespace stdsharp::utility
{
    inline constexpr struct empty_t final
    {
    } empty;

    template<typename T>
    inline constexpr ::std::type_identity<T> type_identity_v{};

    template<typename T>
    concept enumeration = ::std::is_enum_v<T>;

    template<typename T>
    concept reference = ::std::is_reference_v<T>;

    template<typename T>
    concept lvalue_ref = ::std::is_lvalue_reference_v<T>;

    template<typename T>
    concept rvalue_ref = ::std::is_rvalue_reference_v<T>;

    template<typename T>
    concept const_ = ::std::is_const_v<T>;

    template<typename T>
    concept volatile_ = ::std::is_volatile_v<T>;

    template<typename T>
    concept abstract = ::std::is_abstract_v<T>;

    template<typename T>
    concept aggregate = ::std::is_aggregate_v<T>;

    template<typename T>
    concept arithmetic = ::std::is_arithmetic_v<T>;

    template<typename T>
    concept fundamental_array = ::std::is_array_v<T>;

    template<typename T, typename U>
    concept base_of = ::std::is_base_of_v<T, U>;

    template<typename T>
    concept class_ = ::std::is_class_v<T>;

    template<typename T>
    concept function = ::std::is_function_v<T>;

    template<typename T>
    concept pointer = ::std::is_pointer_v<T>;

    template<typename T>
    concept fundamental = ::std::is_fundamental_v<T>;

    template<typename T>
    concept scalar = ::std::is_scalar_v<T>;

    template<typename T>
    concept object = ::std::is_object_v<T>;

    template<typename T>
    concept compound = ::std::is_compound_v<T>;

    template<typename T>
    concept member_object_pointer = ::std::is_member_object_pointer_v<T>;

    template<typename T>
    concept member_function_pointer = ::std::is_member_function_pointer_v<T>;

    template<typename T>
    concept member_pointer = ::std::is_member_pointer_v<T>;

    template<typename T>
    concept polymorphic = ::std::is_polymorphic_v<T>;

    template<typename T>
    concept final = ::std::is_final_v<T>;

    template<typename T>
    concept trivial = ::std::is_trivial_v<T>;

    template<typename T>
    concept trivial_copyable = ::std::is_trivially_copyable_v<T>;

    template<typename T>
    concept trivial_constructible = ::std::is_trivially_constructible_v<T>;

    template<typename T>
    concept trivial_copy_constructible = ::std::is_trivially_copy_constructible_v<T>;

    template<typename T>
    concept trivial_copy_assignable = ::std::is_trivially_copy_assignable_v<T>;

    template<typename T>
    concept trivial_move_constructible = ::std::is_trivially_move_constructible_v<T>;

    template<typename T>
    concept trivial_move_assignable = ::std::is_trivially_move_assignable_v<T>;

    template<typename T, typename U>
    concept not_same_as = !::std::same_as<T, U>;

    template<typename T>
    using add_const_lvalue_ref_t = ::std::add_lvalue_reference_t<::std::add_const_t<T>>;

    template<typename T, typename... Args>
    concept nothrow_constructible_from = ::std::is_nothrow_constructible_v<T, Args...>;

    template<typename T>
    concept nothrow_default_initializable =
        ::std::default_initializable<T> && ::std::is_nothrow_default_constructible_v<T>;

    template<typename T>
    concept nothrow_move_constructible = ::std::is_nothrow_move_constructible_v<T>;

    template<typename T>
    concept nothrow_copy_constructible = ::std::is_nothrow_copy_constructible_v<T>;

    template<typename T, typename U>
    concept nothrow_assignable_from = ::std::is_nothrow_assignable_v<T, U>;

    template<typename T>
    concept nothrow_copy_assignable = ::std::is_nothrow_copy_assignable_v<T>;

    template<typename T>
    concept nothrow_move_assignable = ::std::is_nothrow_move_assignable_v<T>;

    template<typename T>
    concept nothrow_swappable = ::std::is_nothrow_swappable_v<T>;

    template<typename T, typename U>
    concept nothrow_swappable_with = ::std::is_nothrow_swappable_with_v<T, U>;

    template<typename T, typename U>
    concept nothrow_convertible_to = ::std::is_nothrow_convertible_v<T, U>;

    template<typename T, typename U>
    concept convertible_from = ::std::convertible_to<U, T>;

    template<typename T, typename U>
    concept inter_convertible =
        ::std::convertible_to<T, U> && ::stdsharp::utility::convertible_from<T, U>;

    template<typename T, typename U>
    concept nothrow_convertible_from = ::std::is_nothrow_convertible_v<U, T>;

    template<typename T, typename U>
    concept nothrow_inter_convertible = ::stdsharp::utility::nothrow_convertible_to<T, U> &&
        ::stdsharp::utility::nothrow_convertible_from<T, U>;

    template<std::size_t I>
    using index_constant = ::std::integral_constant<::std::size_t, I>;

    template<auto Value>
    struct constant
    {
        using value_type = decltype(Value);

        static constexpr auto value = Value;

        explicit constexpr operator const value_type&() const noexcept { return value; }

        [[nodiscard]] constexpr auto& operator()() const noexcept { return value; }
    };

    template<auto Value>
    inline constexpr auto constant_v = Value;

    template<bool conditional, auto left, auto right>
    inline constexpr auto conditional_v = ::std::conditional_t<
        conditional,
        ::stdsharp::utility::constant<left>,
        ::stdsharp::utility::constant<right> // clang-format off
    >::value; // clang-format on

    template<typename T>
    concept constant_value = requires
    {
        ::std::bool_constant<(std::declval<T>().value, true)>{};
    };

    template<typename Func, typename... Args>
    concept nothrow_invocable = ::std::is_nothrow_invocable_v<Func, Args...>;

    template<typename Func, typename... Args> // clang-format off
    concept invocable_rnonvoid = ::std::invocable<Func, Args...> &&
        ::stdsharp::utility::not_same_as<std::invoke_result_t<Func, Args...>, void>; // clang-format on

    template<typename Func, typename... Args> // clang-format off
    concept nothrow_invocable_rnonvoid = ::stdsharp::utility::nothrow_invocable<Func, Args...> &&
        ::stdsharp::utility::not_same_as<std::invoke_result_t<Func, Args...>, void>; // clang-format on

    // c++23 feature
    template<typename ReturnT, typename Func, typename... Args>
    concept invocable_r = ::std::is_invocable_r_v<ReturnT, Func, Args...>;

    template<typename ReturnT, typename Func, typename... Args>
    concept nothrow_invocable_r = ::std::is_nothrow_invocable_r_v<ReturnT, Func, Args...>;

    template<typename Func, typename... Args>
    concept nothrow_predicate = ::std::predicate<Func, Args...> && //
        ::stdsharp::utility::nothrow_invocable_r<bool, Func, Args...>;

    template<typename T>
    using coerce_t = ::std::invoke_result_t<::ranges::coerce<T>, T&&>;
}
