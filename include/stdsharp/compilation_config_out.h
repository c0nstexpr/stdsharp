#if defined(__GNUG__) || defined(_MSC_VER)
    #pragma pop_macro("STDSHARP_ALWAYS_INLINE")
    #pragma pop_macro("STDSHARP_NO_UNIQUE_ADDRESS")
    #pragma pop_macro("STDSHARP_INTRINSIC")
    #pragma pop_macro("STDSHARP_EBO")
    #pragma pop_macro("STDSHARP_MEM_PACK_IMPL")
    #pragma pop_macro("STDSHARP_MEM_PACK")
#else
    #undef STDSHARP_NO_UNIQUE_ADDRESS
    #undef STDSHARP_INTRINSIC
    #undef STDSHARP_ALWAYS_INLINE
    #undef STDSHARP_EBO
    #undef STDSHARP_MEM_PACK_IMPL
    #undef STDSHARP_MEM_PACK
#endif