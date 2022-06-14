// #include "utility/utility_test.h"
// #include "stdsharp/utility/utility.h"

// namespace stdsharp::test
// {
//     namespace
//     {
//         template<typename T, typename U, typename Expect>
//         struct forward_like_param
//         {
//         };

//         template<typename T, typename U>
//         using copy_const_t = ::std::conditional_t<concepts::const_<T>, const U, U>;
//     }


//     boost::ut::suite& utility_test() noexcept
//     {
//         static boost::ut::suite suite = []
//         {
//             using namespace std;
//             using namespace boost::ut;
//             using namespace bdd;

//             feature("forward_like") =
//                 []<typename T, typename U, typename Expect>(const forward_like_param<T, U, Expect>)
//             {
//                 using res_t = forward_like_t<T, U>;

//                 print( //
//                     fmt::format(
//                         "owner type: {} \tmember type: {}",
//                         reflection::type_name<T>(),
//                         reflection::type_name<U>() // clang-format off
//                     ) // clang-format on
//                 );

//                 static_expect<same_as<res_t, Expect>>() << fmt::format(
//                     "expect type: {} \tactual type: {}",
//                     reflection::type_name<Expect>(),
//                     reflection::type_name<res_t>() //
//                 );
//             } |
//                 tuple{
//                     forward_like_param<int&, int&, int&>{},
//                     forward_like_param<int&, int&&, int&>{},
//                     forward_like_param<int&&, const int&, const int&&>{},
//                     forward_like_param<int&, const int&&, const int&>{},
//                     forward_like_param<const int&, int&, const int&>{},
//                     forward_like_param<const int&&, int&, const int&&>{},
//                 };
//         };
//         return suite;
//     }
// }
