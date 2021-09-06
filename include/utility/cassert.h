#include <cassert>

namespace stdsharp::utility
{
    inline constexpr auto is_debug =
#ifdef NDEBUG
        false
#else
        true
#endif
        ;
}
