//
// Created by BlurringShadow on 2021-9-18.
//

#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace stdsharp::concepts
{
    template<typename T, typename... U>
    concept same_as_any = (::std::same_as<T, U> || ...);

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
    concept const_volatile = const_<T> && volatile_<T>;

    template<typename T>
    concept abstract = ::std::is_abstract_v<T>;

    template<typename T>
    concept aggregate = ::std::is_aggregate_v<T>;

    template<typename T>
    concept character = same_as_any<T, char, char8_t, char16_t, char32_t, wchar_t>;

    template<typename T>
    concept arithmetic = ::std::is_arithmetic_v<T>;

    template<typename T>
    concept signed_ = ::std::is_signed_v<T>;

    template<typename T>
    concept unsigned_ = ::std::is_unsigned_v<T>;

    template<typename T>
    concept floating_point = ::std::is_floating_point_v<T>;

    template<typename T, typename U = T>
    concept arithmetic_like = ::std::three_way_comparable<T> && requires(T t1, U t2)
    { // clang-format off
        { t1 + t2 } -> ::std::same_as<T>;
        { t1 - t2 } -> ::std::same_as<T>;
        { t1 * t2 } -> ::std::same_as<T>;
        { t1 / t2 } -> ::std::same_as<T>;
        { t1 % t2 } -> ::std::same_as<T>;
        { t1 & t2 } -> ::std::same_as<T>;
        { t1 | t2 } -> ::std::same_as<T>;
        { t1 ^ t2 } -> ::std::same_as<T>;
        { t1 << t2 } -> ::std::same_as<T>;
        { t1 >> t2 } -> ::std::same_as<T>;
        { t1 += t2 } -> ::std::same_as<T&>;
        { t1 -= t2 } -> ::std::same_as<T&>;
        { t1 *= t2 } -> ::std::same_as<T&>;
        { t1 /= t2 } -> ::std::same_as<T&>;
        { t1 %= t2 } -> ::std::same_as<T&>;
        { t1 &= t2 } -> ::std::same_as<T&>;
        { t1 |= t2 } -> ::std::same_as<T&>;
        { t1 ^= t2 } -> ::std::same_as<T&>;
        { t1 <<= t2 } -> ::std::same_as<T&>;
        { t1 >>= t2 } -> ::std::same_as<T&>;
    };

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

    template<typename T, typename U>
    concept assignable = ::std::is_assignable_v<T, U>;

    template<typename T, typename U>
    concept assignable_to = ::std::assignable_from<U, T>;

    template<typename T>
    concept move_assignable = ::std::assignable_from<::std::add_lvalue_reference_t<T>, T>;

    template<typename T>
    concept copy_assignable = move_assignable<T> && //
        ::std::assignable_from<::std::add_lvalue_reference_t<T>, ::std::add_const_t<T>> && //
        ::std::assignable_from<
            ::std::add_lvalue_reference_t<T>,
            ::std::add_lvalue_reference_t<T> // clang-format off
        > && // clang-format on
        ::std::assignable_from<
            ::std::add_lvalue_reference_t<T>,
            ::std::add_lvalue_reference_t<::std::add_const_t<T>> // clang-format off
        >; // clang-format on

    template<typename B>
    concept boolean_testable = ::std::convertible_to<B, bool> && requires(B&& b)
    { // clang-format off
        { !::std::forward<B>(b) } -> ::std::convertible_to<bool>; // clang-format on
    };

    template<typename T, typename U>
    concept weakly_equality_comparable_with = // clang-format off
        requires(const ::std::remove_reference_t<T>& t, const ::std::remove_reference_t<U>& u)
        {
            { t == u } -> boolean_testable;
            { t != u } -> boolean_testable;
            { u == t } -> boolean_testable;
            { u != t } -> boolean_testable;
        }; // clang-format on

    template<typename T>
    concept weakly_equality_comparable = weakly_equality_comparable_with<T, T>;

    template<typename T, typename U>
    concept partial_ordered_with = // clang-format off
        requires(const ::std::remove_reference_t<T>& t, const ::std::remove_reference_t<U>& u)
        {
            { t <  u } -> boolean_testable;
            { t >  u } -> boolean_testable;
            { t <= u } -> boolean_testable;
            { t <= u } -> boolean_testable;
            { u <  t } -> boolean_testable;
            { u >  t } -> boolean_testable;
            { u <= t } -> boolean_testable;
            { u <= t } -> boolean_testable;
        }; // clang-format on

    template<typename T, typename... Args>
    concept list_initializable_from = requires
    {
        T{::std::declval<Args>()...};
    };

    template<typename T, typename... Args>
    concept nothrow_list_initializable_from = requires
    {
        requires noexcept(T{::std::declval<Args>()...});
    };

    template<typename T, typename... Args>
    concept implicitly_constructible_from = ::std::constructible_from<T, Args...> && requires
    {
        ::std::declval<void(const T&)>()({::std::declval<Args>()...});
    };

    template<typename T>
    concept implicitly_move_constructible =
        ::std::move_constructible<T> && implicitly_constructible_from<T, T>;

    template<typename T>
    concept implicitly_copy_constructible = ::std::copy_constructible<T> && //
        implicitly_move_constructible<T> && //
        implicitly_constructible_from<T, T&> && //
        implicitly_constructible_from<T, const T> && //
        implicitly_constructible_from<T, const T&>;

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

    template<typename T, typename U>
    concept decay_same_as = ::std::same_as<::std::decay_t<T>, U>;

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
    concept nothrow_assignable = ::std::is_nothrow_assignable_v<T, U>;

    template<typename T, typename U>
    concept nothrow_assignable_from = ::std::is_nothrow_assignable_v<T, U>;

    template<typename T>
    concept nothrow_copy_assignable = ::std::is_nothrow_copy_assignable_v<T>;

    template<typename T>
    concept nothrow_move_assignable = ::std::is_nothrow_move_assignable_v<T>;

    template<typename T>
    concept nothrow_destructible = ::std::is_nothrow_destructible_v<T>;

    template<typename T>
    concept nothrow_swappable = ::std::is_nothrow_swappable_v<T>;

    template<typename T, typename U>
    concept nothrow_swappable_with = ::std::is_nothrow_swappable_with_v<T, U>;

    template<typename T>
    concept nothrow_movable = ::std::movable<T> && //
        nothrow_move_constructible<T> &&
        nothrow_assignable_from<::std::add_lvalue_reference_t<T>, T> && //
        nothrow_swappable<T>;

    template<typename T>
    concept nothrow_copyable = nothrow_movable<T> && //
        nothrow_copy_constructible<T> && //
        nothrow_assignable_from<
            ::std::add_lvalue_reference_t<T>,
            ::std::add_lvalue_reference_t<T> // clang-format off
        > /* clang-format on */ &&
        nothrow_assignable_from<::std::add_lvalue_reference_t<T>, ::std::add_const_t<T>> &&
        nothrow_assignable_from<
            ::std::add_lvalue_reference_t<T>,
            ::std::add_lvalue_reference_t<::std::add_const_t<T>> // clang-format off
        >; // clang-format on

    template<typename T, typename U>
    concept nothrow_convertible_to = ::std::is_nothrow_convertible_v<T, U>;

    template<typename T, typename U>
    concept convertible_from = ::std::convertible_to<U, T>;

    template<typename T, typename U>
    concept inter_convertible = ::std::convertible_to<T, U> && convertible_from<T, U>;

    template<typename T, typename U>
    concept nothrow_convertible_from = ::std::is_nothrow_convertible_v<U, T>;

    template<typename T, typename U>
    concept nothrow_inter_convertible =
        nothrow_convertible_to<T, U> && nothrow_convertible_from<T, U>;

    template<typename Func, typename... Args>
    concept nothrow_invocable = ::std::is_nothrow_invocable_v<Func, Args...>;

    template<typename Func, typename ReturnT, typename... Args>
    concept invocable_r = ::std::is_invocable_r_v<ReturnT, Func, Args...>;

    template<typename Func, typename ReturnT, typename... Args>
    concept nothrow_invocable_r = ::std::is_nothrow_invocable_r_v<ReturnT, Func, Args...>;

    template<typename Func, typename... Args>
    concept nothrow_predicate =
        ::std::predicate<Func, Args...> && nothrow_invocable_r<Func, bool, Args...>;

    template<typename T, typename U>
    concept const_aligned = (::std::is_const_v<T> == ::std::is_const_v<U>);

    template<typename T, typename U>
    concept ref_aligned = ( //
        ::std::is_lvalue_reference_v<T> ?
            ::std::is_lvalue_reference_v<U> :
            (::std::is_rvalue_reference_v<T> == ::std::is_rvalue_reference_v<U>) //
    );

    template<typename T, typename U>
    concept const_ref_aligned = const_aligned<T, U> && ref_aligned<T, U>;

    template<typename T>
    concept nullable_pointer = ::std::default_initializable<T> && //
        ::std::copy_constructible<T> && //
        copy_assignable<T> && //
        ::std::equality_comparable<T> && //
        weakly_equality_comparable_with<T, ::std::nullptr_t>;
}
