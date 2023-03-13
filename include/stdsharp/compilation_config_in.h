#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wconstant-evaluated"
#endif

#ifdef _MSC_VER
    #define STDSHARP_EBO __declspec(empty_bases)
#else
    #define STDSHARP_EBO
#endif


#if _MSC_VER
    #define STDSHARP_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
    #define STDSHARP_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
