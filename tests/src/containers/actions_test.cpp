#include "containers/actions_test.h"
#include "containers/actions.h"

inline namespace
{
    constexpr auto dummy_predicate = [](const auto&) { return true; };

    template<typename T>
    concept vec_req = requires(std::vector<T> v, decltype(v.cbegin()) iter, T value)
    {
        stdsharp::containers::actions::emplace(v, iter, ::std::move(value));
        stdsharp::containers::actions::emplace_back(v, ::std::move(value));
        stdsharp::containers::actions::emplace_front(v, ::std::move(value));

        stdsharp::containers::actions::erase(v, value);
        stdsharp::containers::actions::erase(v, iter);
        stdsharp::containers::actions::erase(v, iter, iter);
        stdsharp::containers::actions::erase_if(v, dummy_predicate);

        stdsharp::containers::actions::pop_front(v);
        stdsharp::containers::actions::pop_back(v);

        stdsharp::containers::actions::resize(v, 0);
    };

    template<typename T>
    concept set_req = requires(std::set<T> v, decltype(v.cbegin()) iter, T value)
    {
        stdsharp::containers::actions::emplace(v, ::std::move(value));

        stdsharp::containers::actions::erase(v, value);
        stdsharp::containers::actions::erase(v, iter);
        stdsharp::containers::actions::erase(v, iter, iter);
        stdsharp::containers::actions::erase_if(v, dummy_predicate);
    };

    template<typename T>
    concept unordered_map_req = requires(
        std::unordered_map<T, int> v,
        decltype(v.cbegin()) iter,
        T value //
    )
    {
        requires(!requires { stdsharp::containers::actions::emplace(v, 0); });

        stdsharp::containers::actions::emplace(v, ::std::move(value), 0);

        stdsharp::containers::actions::erase(v, value);
        stdsharp::containers::actions::erase(v, iter);
        stdsharp::containers::actions::erase(v, iter, iter);
        stdsharp::containers::actions::erase_if(v, dummy_predicate);
    };
}

namespace stdsharp::test::containers::actions
{
    boost::ut::suite& actions_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace std;
            using namespace boost::ut;
            using namespace bdd;
            using namespace stdsharp::containers::actions;

            feature("container actions") = []<typename T>(const type_identity<T>)
            {
                static_expect<vec_req<T>>();
                static_expect<set_req<T>>();
                static_expect<unordered_map_req<T>>();
            } | tuple{type_identity<int>{}, type_identity<unique_ptr<int>>{}};
        };

        return suite;
    }
}
