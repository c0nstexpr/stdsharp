#ifdef STDSHARP_PROLOGUE
    #error "Prologue already defined"
#endif

#define STDSHARP_PROLOGUE

#ifdef _MSC_VER
    #pragma warning(4127)

    #define STDSHARP_DIAGNOSTIC_POP
    #pragma warning(pop)
#elif defined __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdangling-else"
    #pragma GCC diagnostic ignored "-Wlogical-op-parentheses"

    #define STDSHARP_DIAGNOSTIC_POP
    #pragma GCC diagnostic pop
#endif
