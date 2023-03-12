#include "test.h"
#include "stdsharp/memory/object_allocation.h"

using namespace stdsharp;
using namespace std;

template<typename T>
struct t1 : protected T
{
};

template<typename T>
struct t2
{
    template<auto Index>
    struct inner : private t1<T>
    {
    private:
        template<auto I>
            requires(Index == I)
        [[nodiscard]] friend constexpr const T& get(const inner& t) noexcept
        {
            return t;
        }
    };
};

template<typename T, typename U>
struct t3 : t2<T>::template inner<0>, t2<U>::template inner<1>
{
};

void foo()
{
    using arr_t = array<int, 3>;

    const t3<empty_t, arr_t> t_2{};
    auto t2_v = get<0>(t_2);
}

SCENARIO("allocate memory", "[memory][object allocation]") // NOLINT
{
}