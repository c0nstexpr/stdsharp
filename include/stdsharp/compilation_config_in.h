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

#define STDSHARP_MEM_PACK_IMPL(fn_name, impl_name, type, cv, ref)                        \
    constexpr decltype(auto) fn_name(auto&&... args) cv ref noexcept(                    \
        noexcept(type::impl_name(static_cast<cv type ref>(*this), cpp_forward(args)...)) \
    )                                                                                    \
        requires requires {                                                              \
            type::impl_name(static_cast<cv type ref>(*this), cpp_forward(args)...);      \
        }                                                                                \
    {                                                                                    \
        return type::impl_name(static_cast<cv type ref>(*this), cpp_forward(args)...);   \
    }

#define STDSHARP_MEM_PACK(fn_name, impl_name, type)                      \
    STDSHARP_MEM_PACK_IMPL(fn_name, impl_name, type, const volatile, &)  \
    STDSHARP_MEM_PACK_IMPL(fn_name, impl_name, type, const volatile, &&) \
    STDSHARP_MEM_PACK_IMPL(fn_name, impl_name, type, const, &)           \
    STDSHARP_MEM_PACK_IMPL(fn_name, impl_name, type, const, &&)          \
    STDSHARP_MEM_PACK_IMPL(fn_name, impl_name, type, volatile, &)        \
    STDSHARP_MEM_PACK_IMPL(fn_name, impl_name, type, volatile, &&)       \
    STDSHARP_MEM_PACK_IMPL(fn_name, impl_name, type, , &)                \
    STDSHARP_MEM_PACK_IMPL(fn_name, impl_name, type, , &&)