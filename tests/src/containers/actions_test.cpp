// #include "containers/actions_test.h"
// #include "stdsharp/containers/actions.h"

// namespace stdsharp::test::actions
// {
//     using namespace std;
//     using namespace std::ranges;
//     using namespace boost::ut;
//     using namespace bdd;

//     template<typename T>
//     using dummy_predicate_t = bool(const T&);

//     template<typename T>
//     concept vec_req = requires(
//         vector<T> v,
//         iterator_t<vector<T>> iter,
//         T value,
//         dummy_predicate_t<T> dummy_predicate //
//     )
//     {
//         stdsharp::actions::emplace(v, iter, std::move(value));
//         stdsharp::actions::emplace_back(v, std::move(value));
//         stdsharp::actions::emplace_front(v, std::move(value));

//         stdsharp::actions::erase(v, value);
//         stdsharp::actions::erase(v, iter);
//         stdsharp::actions::erase(v, iter, iter);
//         stdsharp::actions::erase_if(v, dummy_predicate);

//         stdsharp::actions::pop_front(v);
//         stdsharp::actions::pop_back(v);

//         stdsharp::actions::resize(v, 0);
//     };

//     template<typename T>
//     concept set_req = requires(
//         set<T> v,
//         iterator_t<set<T>> iter,
//         T value,
//         dummy_predicate_t<T> dummy_predicate //
//     )
//     {
//         stdsharp::actions::emplace(v, std::move(value));

//         stdsharp::actions::erase(v, value);
//         stdsharp::actions::erase(v, iter);
//         stdsharp::actions::erase(v, iter, iter);
//         stdsharp::actions::erase_if(v, dummy_predicate);
//     };

//     template<typename T>
//     concept unordered_map_req = requires(
//         unordered_map<T, int> v,
//         iterator_t<unordered_map<T, int>> iter,
//         T value,
//         dummy_predicate_t<pair<const T, int>> dummy_predicate //
//     )
//     {
//         stdsharp::actions::emplace(v, std::move(value), 0);

//         stdsharp::actions::erase(v, value);
//         stdsharp::actions::erase(v, iter);
//         stdsharp::actions::erase(v, iter, iter);
//         stdsharp::actions::erase_if(v, dummy_predicate);
//     };

//     void vector_actions_test()
//     {
//         feature("vector actions") = []<typename T>(const type_identity<T>)
//         {
//             println(fmt::format("current type {}", reflection::type_name<T>()));
//             static_expect<vec_req<T>>();
//         } | tuple{type_identity<int>{}, type_identity<unique_ptr<float>>{}};

//         feature("vector concept checking") = []
//         {
//             using vec = std::vector<int>;

//             println(fmt::format("current type {}", reflection::type_name<vec>()));
//             static_expect<containers::sequence_container<vec>>();
//             static_expect<!containers::associative_container<vec>>();
//             static_expect<!containers::unordered_associative_container<vec>>();
//         };
//     }

//     void set_actions_test()
//     {
//         feature("set actions") = []<typename T>(const type_identity<T>)
//         {
//             println(fmt::format("current type {}", reflection::type_name<T>()));
//             static_expect<set_req<T>>();
//         } | tuple{type_identity<int>{}, type_identity<unique_ptr<float>>{}};

//         feature("set concept checking") = []
//         {
//             using set = std::set<int>;

//             println(fmt::format("current type {}", reflection::type_name<set>()));
//             static_expect<containers::associative_container<set>>();
//             static_expect<!containers::sequence_container<set>>();
//         };
//     }

//     void unordered_map_actions_test()
//     {
//         feature("unordered map actions") = []<typename T>(const type_identity<T>)
//         {
//             println(fmt::format("current type {}", reflection::type_name<T>()));
//             static_expect<unordered_map_req<T>>();
//         } | tuple{type_identity<int>{}, type_identity<unique_ptr<long>>{}};

//         feature("unordered map concept checking") = []
//         {
//             using map = std::unordered_map<int, int>;

//             println(fmt::format("current type {}", reflection::type_name<map>()));
//             static_expect<containers::unordered_associative_container<map>>();
//             static_expect<!containers::sequence_container<map>>();
//         };
//     }

//     boost::ut::suite& actions_test()
//     {
//         static boost::ut::suite suite = []
//         {
//             vector_actions_test();
//             set_actions_test();
//             unordered_map_actions_test();
//         };

//         return suite;
//     }
// }