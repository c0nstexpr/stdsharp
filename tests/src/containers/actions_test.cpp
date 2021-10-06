#include "containers/actions_test.h"
#include "containers/actions.h"

namespace stdsharp::test::containers::actions
{
    constexpr auto dummy_predicate = [](const auto&) { return true; };

    template<typename T>
    concept vec_req = requires(std::vector<T> v, T value)
    {
        stdsharp::containers::actions::emplace(v, v.begin(), ::std::move(value));
        stdsharp::containers::actions::emplace_back(v, ::std::move(value));
        stdsharp::containers::actions::emplace_front(v, ::std::move(value));

        stdsharp::containers::actions::erase(v, value);
        stdsharp::containers::actions::erase(v, v.begin());
        stdsharp::containers::actions::erase_if(v, dummy_predicate);

        stdsharp::containers::actions::pop_front(v);
        stdsharp::containers::actions::pop_back(v);

        stdsharp::containers::actions::resize(v, 0);
    };

    boost::ut::suite& actions_test()
    {
        static boost::ut::suite suite = []
        {
            using namespace std;
            using namespace boost::ut;
            using namespace bdd;
            using namespace stdsharp::containers::actions;

            vector<int> v{};

            feature("container actions") = []<typename T>(const type_identity<T>) //
            {
                static_expect<vec_req<T>>(); //
            } | tuple{type_identity<int>{}, type_identity<unique_ptr<int>>{}};
        };

        return suite;
    }
}
