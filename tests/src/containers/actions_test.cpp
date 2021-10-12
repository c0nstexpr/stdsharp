#include "containers/actions_test.h"
#include "containers/actions.h"
#include "type_traits/object.h"

namespace stdsharp::test::containers::actions
{
    using namespace std;
    using namespace std::ranges;
    using namespace boost::ut;
    using namespace bdd;
    using namespace stdsharp::functional;
    using namespace stdsharp::ranges;
    using namespace stdsharp::containers;
    using namespace stdsharp::containers::actions;

    namespace
    {
        template<typename T>
        using dummy_predicate_t = bool(const T&);

        template<typename T>
        concept vec_req = requires(
            vector<T> v,
            decltype(v.cbegin()) iter,
            T value,
            dummy_predicate_t<T> dummy_predicate //
        )
        {
            stdsharp::containers::actions::emplace(v, iter, std::move(value));
            stdsharp::containers::actions::emplace_back(v, std::move(value));
            stdsharp::containers::actions::emplace_front(v, std::move(value));

            stdsharp::containers::actions::erase(v, value);
            stdsharp::containers::actions::erase(v, iter);
            stdsharp::containers::actions::erase(v, iter, iter);
            stdsharp::containers::actions::erase_if(v, dummy_predicate);

            stdsharp::containers::actions::pop_front(v);
            stdsharp::containers::actions::pop_back(v);

            stdsharp::containers::actions::resize(v, 0);
        };

        template<typename T>
        concept set_req = requires(
            set<T> v,
            decltype(v.cbegin()) iter,
            T value,
            dummy_predicate_t<T> dummy_predicate //
        )
        {
            stdsharp::containers::actions::emplace(v, std::move(value));

            stdsharp::containers::actions::erase(v, value);
            stdsharp::containers::actions::erase(v, iter);
            stdsharp::containers::actions::erase(v, iter, iter);
            stdsharp::containers::actions::erase_if(v, dummy_predicate);
        };

        template<typename T>
        concept unordered_map_req = requires(
            unordered_map<T, int> v,
            decltype(v.cbegin()) iter,
            T value,
            dummy_predicate_t<pair<const T, int>> dummy_predicate //
        )
        {
            stdsharp::containers::actions::emplace(v, std::move(value), 0);

            stdsharp::containers::actions::erase(v, value);
            stdsharp::containers::actions::erase(v, iter);
            stdsharp::containers::actions::erase(v, iter, iter);
            stdsharp::containers::actions::erase_if(v, dummy_predicate);
        };

        void vector_actions_test()
        {
            feature("vector actions") = []<typename T>(const type_identity<T>)
            {
                println(fmt::format("current type {}", reflection::type_name<T>()));
                static_expect<vec_req<T>>();
            } | tuple{type_identity<int>{}, type_identity<unique_ptr<float>>{}};

            struct range_as_iterators_params
            {
                vector<int> initial_v_list;
                initializer_list<int> expected_v_list;
            };

            // clang-format off
            // NOLINTNEXTLINE(performance-unnecessary-value-param)
            feature("range as iterators") = [](range_as_iterators_params params) // clang-format on
            {
                auto& [v_list, expected_v_list] = params;

                const auto unique_op = [&v_list = v_list]
                {
                    stdsharp::containers::actions::erase(
                        v_list,
                        ((v_list | decompose_to(rng_as_iters)) | ::ranges::unique)(),
                        v_list.cend() //
                    );
                };

                println(fmt::format("current value: {}", v_list));

                unique_op();

                println( //
                    fmt::format(
                        "after first unique operation, values are: {}",
                        v_list // clang-format off
                    ) // clang-format on
                );

                ((v_list | decompose_to(rng_as_iters)) | ::ranges::sort)();

                println( //
                    fmt::format(
                        "after sort, values are: {}",
                        v_list // clang-format off
                    ) // clang-format on
                );

                unique_op();

                expect( //
                    std::ranges::equal(expected_v_list, v_list) // clang-format off
                ) << fmt::format("actual values are: {}",v_list);
            } | tuple{
                range_as_iterators_params{
                    {1, 2, 1, 1, 3, 3, 3, 4, 5, 4},
                    {1, 2, 3, 4, 5}
                }
            }; // clang-format on
        }

        void set_actions_test()
        {
            feature("set actions") = []<typename T>(const type_identity<T>)
            {
                println(fmt::format("current type {}", reflection::type_name<T>()));
                static_expect<set_req<T>>();
            } | tuple{type_identity<int>{}};
        }

        void unordered_map_actions_test()
        {
            feature("unordered map actions") = []<typename T>(const type_identity<T>)
            {
                println(fmt::format("current type {}", reflection::type_name<T>()));
                static_expect<unordered_map_req<T>>();
            } | tuple{type_identity<int>{}, type_identity<unique_ptr<long>>{}};
        }
    }

    boost::ut::suite& actions_test()
    {
        static boost::ut::suite suite = []
        {
            vector_actions_test();
            set_actions_test();
            unordered_map_actions_test();
        };

        return suite;
    }
}
