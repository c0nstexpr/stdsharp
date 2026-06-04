#if defined(__GNUG__)
    #define STDSHARP_ALWAYS_INLINE [[gnu::always_inline]]
#elif defined(_MSC_VER)
    #define STDSHARP_ALWAYS_INLINE [[msvc::forceinline]]
#else
    #define STDSHARP_ALWAYS_INLINE inline
#endif

#if defined(_MSC_VER)
    #define STDSHARP_INTRINSIC [[msvc::intrinsic]] [[nodiscard]] static
#else
    #define STDSHARP_INTRINSIC [[nodiscard]] STDSHARP_ALWAYS_INLINE static
#endif

#if defined(_MSC_VER) || (defined(_WIN32) && defined(__clang__))
    #define STDSHARP_EBO __declspec(empty_bases)
#else
    #define STDSHARP_EBO
#endif
