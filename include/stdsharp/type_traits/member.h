// Created by BlurringShadow at 2021-03-05-下午 11:53

#pragma once

#include "function.h"

namespace stdsharp::type_traits
{
    template<typename>
    struct member_traits;

    template<typename T, typename ClassT>
    struct member_traits<T(ClassT::*)> : ::std::type_identity<T>
    {
        using class_t = ClassT;
    };
    template<auto Ptr>
    struct member_pointer_traits : member_traits<::std::decay_t<decltype(Ptr)>>
    {
    };

    template<auto Ptr>
    using member_t = typename member_pointer_traits<Ptr>::type;

    template<typename>
    struct member_function_traits;

    enum class member_ref_qualifier
    {
        none,
        lvalue,
        rvalue
    };

    template<typename T, typename ClassT>
    concept member_of = ::std::same_as<typename member_traits<T>::class_t, ClassT>;

    template<typename T, typename ClassT>
    concept member_func_of = ::std::is_member_pointer_v<T> && ::std::same_as<
        typename //
        member_function_traits<T>::class_t,
        ClassT // clang-format off
    >; // clang-format on


    template<auto Ptr>
    struct member_function_pointer_traits : member_function_traits<::std::decay_t<decltype(Ptr)>>
    {
    };

#define UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS(const_, volatile_, ref_, noexcept_, qualifiers)     \
    template<typename R, typename ClassT, typename... Args>                                       \
    struct member_function_traits<R (ClassT::*)(Args...) qualifiers> :                            \
        function_traits<::std::conditional_t<noexcept_, R (*)(Args...) noexcept, R (*)(Args...)>> \
    {                                                                                             \
        using class_t = ClassT;                                                                   \
        static constexpr auto is_const = const_;                                                  \
        static constexpr auto is_volatile = volatile_;                                            \
        static constexpr auto ref_type = member_ref_qualifier::ref_;                              \
    };

#define UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_CONST_PACK(               \
    is_volatile, ref_type, is_noexcept, qualifiers /**/                 \
)                                                                       \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS(                              \
        true, is_volatile, ref_type, is_noexcept, const qualifiers /**/ \
    )                                                                   \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS(false, is_volatile, ref_type, is_noexcept, qualifiers)

#define UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(ref_type, is_noexcept, qualifiers) \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_CONST_PACK(                                          \
        true, ref_type, is_noexcept, volatile qualifiers /**/                                  \
    )                                                                                          \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_CONST_PACK(false, ref_type, is_noexcept, qualifiers)

#define UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_REF_PACK(is_noexcept, qualifiers)           \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(none, is_noexcept, qualifiers)    \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(lvalue, is_noexcept, &qualifiers) \
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(rvalue, is_noexcept, &&qualifiers)

    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_REF_PACK(true, noexcept)
    UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_REF_PACK(false, )

#undef UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_REF_PACK
#undef UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK
#undef UTILITY_TRAITS_MEMBER_FUNCTION_TRAITS_CONST_PACK
}
