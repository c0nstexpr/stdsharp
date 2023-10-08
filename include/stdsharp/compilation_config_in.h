#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wconstant-evaluated"
#endif

#ifdef __GNUG__
    #define STDSHARP_ALWAYS_INLINE [[gnu::always_inline]]
#else
    #define STDSHARP_ALWAYS_INLINE inline
#endif

#ifdef _MSC_VER
    #define STDSHARP_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
    #define STDSHARP_INTRINSIC [[msvc::intrinsic]]
#else
    #define STDSHARP_NO_UNIQUE_ADDRESS [[no_unique_address]]
    #define STDSHARP_INTRINSIC STDSHARP_ALWAYS_INLINE
#endif

#if defined(_MSC_VER) || (defined(_WIN32) && defined(__clang__))
    #define STDSHARP_EBO __declspec(empty_bases)
#else
    #define STDSHARP_EBO
#endif

#if __has_cpp_attribute(assume)
    #define STDSHARP_ASSUME(...) [[assume(__VA_ARGS__)]]
#elif defined(_MSC_VER)
    #define STDSHARP_ASSUME(...) __assume(__VA_ARGS__)
#else
    #define STDSHARP_ASSUME(...)
#endif

#define STDSHARP_MEM_PACK_IMPL(fn_name, cv, ref)                                                 \
    template<typename... Args, typename This = cv t ref>                                         \
    constexpr decltype(auto) fn_name(Args&&... args                                              \
    ) noexcept(noexcept(t::fn_name##_impl(static_cast<This>(*this), cpp_forward(args)...)))      \
        requires requires { t::fn_name##_impl(static_cast<This>(*this), cpp_forward(args)...); } \
    {                                                                                            \
        return t::fn_name##_impl(static_cast<This>(*this), cpp_forward(args)...);                \
    }

#define STDSHARP_MEM_PACK(fn_name)                      \
    STDSHARP_MEM_PACK_IMPL(fn_name, const volatile, &)  \
    STDSHARP_MEM_PACK_IMPL(fn_name, const volatile, &&) \
    STDSHARP_MEM_PACK_IMPL(fn_name, const, &)           \
    STDSHARP_MEM_PACK_IMPL(fn_name, const, &&)          \
    STDSHARP_MEM_PACK_IMPL(fn_name, volatile, &)        \
    STDSHARP_MEM_PACK_IMPL(fn_name, volatile, &&)       \
    STDSHARP_MEM_PACK_IMPL(fn_name, , &)                \
    STDSHARP_MEM_PACK_IMPL(fn_name, , &&)