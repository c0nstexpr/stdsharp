#include "containers/containers_test.h"
#include "containers/containers.h"

namespace stdsharp::test::containers
{
    boost::ut::suite& containers_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace std;
            using namespace boost::ut;
            using namespace bdd;
            using namespace stdsharp::containers;

            feature("container concept") = []<typename T>(const type_identity<T>)
            {
                using vec = vector<T>;

                println(fmt::format("current type {}", reflection::type_name<T>()));

                static_expect<sequence_container<vec>>();
                static_expect<contiguous_container<vec>>();

                static_expect<associative_container<set<T>>>();
                static_expect<associative_container<map<T, T>>>();
                static_expect<associative_container<multiset<T>>>();
                static_expect<associative_container<multimap<T, T>>>();

                constexpr auto associative_vec = !associative_container<vec>;

                static_expect<associative_vec>();

                static_expect<unordered_associative_container<unordered_set<T>>>();
                static_expect<unordered_associative_container<unordered_map<T, T>>>();
                static_expect<unordered_associative_container<unordered_multiset<T>>>();
                static_expect<unordered_associative_container<unordered_multimap<T, T>>>();
            } | tuple{type_identity<int>{}, type_identity<unique_ptr<int>>{}};
        };

        return suite;
    }
}
