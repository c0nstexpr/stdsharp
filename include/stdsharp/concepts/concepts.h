//

//
#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <tuple>

#include "../macros.h"
#include "../namespace_alias.h"

namespace stdsharp
{
    template<typename T, typename... U>
    concept same_as_any = (std::same_as<T, U> || ...);

    template<typename T, typename... U>
    concept all_same = (std::same_as<T, U> && ...);

    template<typename T>
    concept enum_ = std::is_enum_v<T>;

    template<typename T>
    concept reference = std::is_reference_v<T>;

    template<typename T>
    concept lvalue_ref = std::is_lvalue_reference_v<T>;

    template<typename T>
    concept rvalue_ref = std::is_rvalue_reference_v<T>;

    template<typename T>
    concept const_ = std::is_const_v<T>;

    template<typename T>
    concept non_const = !const_<T>;

    template<typename T>
    concept const_lvalue_ref = std::is_lvalue_reference_v<T> && const_<std::remove_reference_t<T>>;

    template<typename T>
    concept volatile_ = std::is_volatile_v<T>;

    template<typename T>
    concept const_volatile = const_<T> && volatile_<T>;

    template<typename T>
    concept abstract = std::is_abstract_v<T>;

    template<typename T>
    concept aggregate = std::is_aggregate_v<T>;

    template<typename T>
    concept character = same_as_any<T, char, char8_t, char16_t, char32_t, wchar_t>;

    template<typename T>
    concept arithmetic = std::is_arithmetic_v<T>;

    template<typename T>
    concept signed_ = std::is_signed_v<T>;

    template<typename T>
    concept empty_type = std::is_empty_v<T>;

    template<typename T>
    concept unsigned_ = std::is_unsigned_v<T>;

    template<typename T>
    concept floating_point = std::is_floating_point_v<T>;

    template<typename T, typename U = T>
    concept arithmetic_like = std::three_way_comparable<T> && requires(T t1, U t2) {
        {
            t1 + t2
        } -> std::same_as<T>;
        {
            t1 - t2
        } -> std::same_as<T>;
        {
            t1* t2
        } -> std::same_as<T>;
        {
            t1 / t2
        } -> std::same_as<T>;
        {
            t1 % t2
        } -> std::same_as<T>;
        {
            t1& t2
        } -> std::same_as<T>;
        {
            t1 | t2
        } -> std::same_as<T>;
        {
            t1 ^ t2
        } -> std::same_as<T>;
        {
            t1 << t2
        } -> std::same_as<T>;
        {
            t1 >> t2
        } -> std::same_as<T>;
        {
            t1 += t2
        } -> std::same_as<T&>;
        {
            t1 -= t2
        } -> std::same_as<T&>;
        {
            t1 *= t2
        } -> std::same_as<T&>;
        {
            t1 /= t2
        } -> std::same_as<T&>;
        {
            t1 %= t2
        } -> std::same_as<T&>;
        {
            t1 &= t2
        } -> std::same_as<T&>;
        {
            t1 |= t2
        } -> std::same_as<T&>;
        {
            t1 ^= t2
        } -> std::same_as<T&>;
        {
            t1 <<= t2
        } -> std::same_as<T&>;
        {
            t1 >>= t2
        } -> std::same_as<T&>;
    };

    template<typename T>
    concept fundamental_array = std::is_array_v<T>;

    template<typename T, typename U>
    concept base_of = std::is_base_of_v<T, U>;

    template<typename T>
    concept class_ = std::is_class_v<T>;

    template<typename T>
    concept function = std::is_function_v<T>;

    template<typename T>
    concept pointer = std::is_pointer_v<T>;

    template<typename T>
    concept fundamental = std::is_fundamental_v<T>;

    template<typename T>
    concept scalar = std::is_scalar_v<T>;

    template<typename T>
    concept object = std::is_object_v<T>;

    template<typename T>
    concept compound = std::is_compound_v<T>;

    template<typename T>
    concept member_object_pointer = std::is_member_object_pointer_v<T>;

    template<typename T>
    concept member_function_pointer = std::is_member_function_pointer_v<T>;

    template<typename T>
    concept member_pointer = std::is_member_pointer_v<T>;

    template<typename T>
    concept polymorphic = std::is_polymorphic_v<T>;

    template<typename T>
    concept final = std::is_final_v<T>;

    template<typename T>
    concept trivial = std::is_trivial_v<T>;

    template<typename T>
    concept trivial_copyable = std::is_trivially_copyable_v<T>;

    template<typename T>
    concept trivial_constructible = std::is_trivially_constructible_v<T>;

    template<typename T>
    concept trivial_copy_constructible = std::is_trivially_copy_constructible_v<T>;

    template<typename T>
    concept trivial_copy_assignable = std::is_trivially_copy_assignable_v<T>;

    template<typename T>
    concept trivial_move_constructible = std::is_trivially_move_constructible_v<T>;

    template<typename T>
    concept trivial_move_assignable = std::is_trivially_move_assignable_v<T>;

    template<typename T, typename U>
    concept assignable = std::is_assignable_v<T, U>;

    template<typename T, typename U>
    concept nothrow_assignable = std::is_nothrow_assignable_v<T, U>;

    template<typename T>
    concept move_assignable = std::is_move_assignable_v<T>;

    template<typename T>
    concept copy_assignable = std::is_copy_assignable_v<T>;

    template<typename T, typename U>
    concept nothrow_convertible_to = std::is_nothrow_convertible_v<T, U>;

    template<typename T>
    concept inheritable = class_<T> && !final<T>;

    template<typename T, typename U>
    concept assignable_to = assignable<U, T>;

    template<typename From, typename To>
    concept explicitly_convertible = requires(From&& v) { static_cast<To>(cpp_forward(v)); };

    template<typename From, typename To>
    concept nothrow_explicitly_convertible =
        requires(From&& v) { requires noexcept(static_cast<To>(cpp_forward(v))); };

    template<typename To, typename From>
    concept explicitly_convertible_from = explicitly_convertible<From, To>;

    template<typename To, typename From>
    concept nothrow_explicitly_convertible_from = nothrow_explicitly_convertible<From, To>;

    template<typename T, typename U>
    concept convertible_from = std::convertible_to<U, T>;

    template<typename T, typename U>
    concept nothrow_convertible_from = std::is_nothrow_convertible_v<U, T>;

    template<typename T, typename U>
    concept inter_convertible = std::convertible_to<T, U> && convertible_from<T, U>;

    template<typename T, typename U>
    concept nothrow_inter_convertible =
        nothrow_convertible_to<T, U> && nothrow_convertible_from<T, U>;

    template<typename T, typename U>
    concept explicitly_inter_convertible =
        explicitly_convertible<T, U> && explicitly_convertible_from<T, U>;

    template<typename T, typename U>
    concept nothrow_explicitly_inter_convertible =
        nothrow_explicitly_convertible<T, U> && nothrow_explicitly_convertible_from<T, U>;

    template<typename B>
    concept boolean_testable = std::convertible_to<B, bool> && requires(B&& b) {
        {
            !cpp_forward(b)
        } -> std::convertible_to<bool>;
    };

    template<typename B>
    concept nothrow_boolean_testable = nothrow_convertible_to<B, bool> && requires(B&& b) {
        {
            !cpp_forward(b)
        } noexcept -> nothrow_convertible_to<bool>;
    };

    template<typename T, typename U>
    concept weakly_equality_comparable_with =
        requires(const std::remove_reference_t<T>& t, const std::remove_reference_t<U>& u) {
            {
                t == u
            } -> boolean_testable;
            {
                t != u
            } -> boolean_testable;
            {
                u == t
            } -> boolean_testable;
            {
                u != t
            } -> boolean_testable;
        };

    template<typename T, typename U>
    concept nothrow_weakly_equality_comparable_with =
        requires(const std::remove_reference_t<T>& t, const std::remove_reference_t<U>& u) {
            {
                t == u
            } noexcept -> nothrow_boolean_testable;
            {
                t != u
            } noexcept -> nothrow_boolean_testable;
            {
                u == t
            } noexcept -> nothrow_boolean_testable;
            {
                u != t
            } noexcept -> nothrow_boolean_testable;
        };

    template<typename T>
    concept weakly_equality_comparable = weakly_equality_comparable_with<T, T>;

    template<typename T>
    concept nothrow_weakly_equality_comparable = nothrow_weakly_equality_comparable_with<T, T>;

    template<typename T, typename U>
    concept partial_ordered_with =
        requires(const std::remove_reference_t<T>& t, const std::remove_reference_t<U>& u) {
            {
                t < u
            } -> boolean_testable;
            {
                t > u
            } -> boolean_testable;
            {
                t <= u
            } -> boolean_testable;
            {
                t <= u
            } -> boolean_testable;
            {
                u < t
            } -> boolean_testable;
            {
                u > t
            } -> boolean_testable;
            {
                u <= t
            } -> boolean_testable;
            {
                u <= t
            } -> boolean_testable;
        };

    template<typename T, typename... Args>
    concept list_initializable_from = requires { T{std::declval<Args>()...}; };

    template<typename T, typename... Args>
    concept nothrow_list_initializable_from =
        requires { requires noexcept(T{std::declval<Args>()...}); };

    template<typename T, typename... Args>
    concept implicitly_constructible_from = std::constructible_from<T, Args...> &&
        requires { std::declval<void(const T&)>()({std::declval<Args>()...}); };

    template<typename T>
    concept implicitly_move_constructible =
        std::move_constructible<T> && implicitly_constructible_from<T, T>;

    template<typename T>
    concept implicitly_copy_constructible = std::copy_constructible<T> && //
        implicitly_move_constructible<T> && //
        implicitly_constructible_from<T, T&> && //
        implicitly_constructible_from<T, const T> && //
        implicitly_constructible_from<T, const T&>;

    template<typename T, typename U>
    concept not_same_as = !std::same_as<T, U>;

    template<typename T, typename U>
    concept decay_same_as = std::same_as<std::decay_t<T>, U>;

    template<typename T>
    concept decay_constructible = std::constructible_from<std::decay_t<T>, T>;

    template<typename T, typename... Args>
    concept nothrow_constructible_from = std::is_nothrow_constructible_v<T, Args...>;

    template<typename T>
    concept nothrow_decay_constructible = nothrow_constructible_from<std::decay_t<T>, T>;

    template<typename T>
    concept nothrow_default_initializable =
        std::default_initializable<T> && std::is_nothrow_default_constructible_v<T>;

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

    template<typename T, typename U>
    concept nothrow_swappable_with =
        requires(T& t, U& u) { requires noexcept(std::ranges::swap(t, u)); };

    template<typename T>
    concept nothrow_swappable = nothrow_swappable_with<T, T>;

    template<typename T>
    concept nothrow_movable = std::movable<T> && //
        nothrow_move_constructible<T> &&
        nothrow_assignable_from<std::add_lvalue_reference_t<T>, T> && //
        nothrow_swappable<T>;

    template<typename T>
    concept nothrow_copyable = requires(
        std::add_lvalue_reference_t<T> ref,
        std::add_const_t<T> const_,
        std::add_lvalue_reference_t<decltype(const_)> const_ref
    ) {
        requires nothrow_movable<T>;
        requires nothrow_copy_constructible<T>;
        requires nothrow_assignable_from<decltype(ref), decltype(ref)>;
        requires nothrow_assignable_from<decltype(ref), decltype(const_)>;
        requires nothrow_assignable_from<decltype(ref), decltype(const_ref)>;
    };

    template<typename Func, typename... Args>
    concept nothrow_invocable = std::is_nothrow_invocable_v<Func, Args...>;

    template<typename Func, typename ReturnT, typename... Args>
    concept invocable_r = std::is_invocable_r_v<ReturnT, Func, Args...>;

    template<typename Func, typename ReturnT, typename... Args>
    concept nothrow_invocable_r = std::is_nothrow_invocable_r_v<ReturnT, Func, Args...>;

    template<typename Func, typename... Args>
    concept nothrow_predicate = nothrow_invocable_r<Func, bool, Args...>;

    template<typename T, typename U>
    concept const_aligned = (const_<T> == const_<U>);

    template<typename T, typename U>
    concept ref_aligned = (lvalue_ref<T> == lvalue_ref<U>)&&(rvalue_ref<T> == rvalue_ref<U>);

    template<typename T, typename U>
    concept const_ref_aligned = const_aligned<T, U> && ref_aligned<T, U>;

    template<typename T>
    concept nullable_pointer = std::default_initializable<T> && //
        std::copy_constructible<T> && //
        copy_assignable<T> && //
        std::equality_comparable<T> && //
        weakly_equality_comparable_with<T, std::nullptr_t>;

    template<typename T>
    concept referenceable = requires(T&) { 0; };

    template<typename T>
    concept dereferenceable = requires(T& t) {
        {
            *t
        } -> referenceable;
    };

    template<typename T>
    concept nothrow_dereferenceable = dereferenceable<T> && requires(T& t) {
        {
            *t
        } noexcept;
    };

    template<typename T, template<typename...> typename Impl, typename... U>
    concept proxy_concept = Impl<T, U...>::value;

    template<typename T, template<typename...> typename Impl, typename... U>
    concept concept_not = (!Impl<T, U...>::value);
}