#include <cassert>

namespace std_sharp::utility
{
    inline constexpr auto is_debug =
#ifdef NDEBUG
        false
#else
        true
#endif
        ;
}
