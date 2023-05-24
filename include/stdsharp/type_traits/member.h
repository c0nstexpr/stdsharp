
#pragma once

#include "function.h"

namespace stdsharp
{
    template<typename>
    struct member_traits;

    template<typename T, typename ClassT>
    struct member_traits<T ClassT::*>
    {
        using type = T;
        using class_t = ClassT;
    };

    template<auto Ptr>
    using member_pointer_traits = member_traits<decltype(Ptr)>;

    template<auto Ptr>
    using member_t = ::meta::_t<member_pointer_traits<Ptr>>;

    namespace details
    {
        template<typename, typename, bool, bool, ref_qualifier, bool, typename...>
        struct member_function_traits;
    }

#define STDSHARP_MEMBER_FUNCTION_TRAITS(const_, volatile_, ref_, qualifiers)                      \
    namespace details                                                                             \
    {                                                                                             \
        template<typename R, typename ClassT, bool Noexcept, typename... Args>                    \
        struct member_function_traits<                                                            \
            R,                                                                                    \
            ClassT,                                                                               \
            const_,                                                                               \
            volatile_,                                                                            \
            ref_qualifier::ref_,                                                                  \
            Noexcept,                                                                             \
            Args...> : function_traits<R (*)(Args...) noexcept(Noexcept)>                         \
        {                                                                                         \
            static constexpr auto is_const = const_;                                              \
            static constexpr auto is_volatile = volatile_;                                        \
            static constexpr auto ref_type = ref_qualifier::ref_;                                 \
            using class_t = ClassT;                                                               \
            using qualified_class_t = apply_qualifiers<class_t, is_const, is_volatile, ref_type>; \
        };                                                                                        \
    }                                                                                             \
                                                                                                  \
    template<typename R, typename ClassT, typename... Args>                                       \
    struct member_traits<R ClassT::*(Args...)qualifiers noexcept> :                               \
        details::member_function_traits<                                                          \
            R,                                                                                    \
            ClassT,                                                                               \
            const_,                                                                               \
            volatile_,                                                                            \
            ref_qualifier::ref_,                                                                  \
            true,                                                                                 \
            Args...>                                                                              \
    {                                                                                             \
    };                                                                                            \
                                                                                                  \
    template<typename R, typename ClassT, typename... Args>                                       \
    struct member_traits<R (ClassT::*)(Args...) qualifiers> :                                     \
        details::member_function_traits<                                                          \
            R,                                                                                    \
            ClassT,                                                                               \
            const_,                                                                               \
            volatile_,                                                                            \
            ref_qualifier::ref_,                                                                  \
            false,                                                                                \
            Args...>                                                                              \
    {                                                                                             \
    };

#define STDSHARP_MEMBER_FUNCTION_TRAITS_CONST_PACK(is_volatile, ref_type, qualifiers) \
    STDSHARP_MEMBER_FUNCTION_TRAITS(true, is_volatile, ref_type, const qualifiers)    \
    STDSHARP_MEMBER_FUNCTION_TRAITS(false, is_volatile, ref_type, qualifiers)

#define STDSHARP_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(ref_type, qualifiers)         \
    STDSHARP_MEMBER_FUNCTION_TRAITS_CONST_PACK(true, ref_type, volatile qualifiers) \
    STDSHARP_MEMBER_FUNCTION_TRAITS_CONST_PACK(false, ref_type, qualifiers)

    STDSHARP_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(none, )
    STDSHARP_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(lvalue, &)
    STDSHARP_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(rvalue, &&)

#undef STDSHARP_MEMBER_FUNCTION_TRAITS_REF_PACK
#undef STDSHARP_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK
#undef STDSHARP_MEMBER_FUNCTION_TRAITS_CONST_PACK

    template<typename T, typename ClassT>
    concept member_of = ::std::same_as<typename member_traits<T>::class_t, ClassT>;

    template<typename T, typename ClassT>
    concept member_func_of =
        ::std::is_member_pointer_v<T> && ::std::same_as<typename member_traits<T>::class_t, ClassT>;
}