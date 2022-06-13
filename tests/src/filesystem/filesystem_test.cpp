// #include "stdsharp/filesystem/filesystem.h"
// #include "filesystem/filesystem_test.h"

// #if __cpp_lib_format >= 201907L
//     #define FORMAT_NS ::std
// #else
//     #define FORMAT_NS ::fmt
// #endif


// namespace stdsharp::test::filesystem
// {
//     boost::ut::suite& filesystem_test()
//     {
//         static boost::ut::suite suite = []
//         {
//             using namespace std;
//             using boost::ut::log;
//             using boost::ut::static_expect;
//             using boost::ut::expect;
//             using boost::ut::eq;
//             using namespace boost::ut::bdd;
//             using namespace stdsharp::filesystem;

//             feature("space size") = []
//             {
//                 static_expect<default_initializable<bytes>>()
//                     << FORMAT_NS::format("space size should be constructbile by default");

//                 static_expect<invocable<::std::plus<>, bytes, int>>()
//                     << FORMAT_NS::format("space size should be able to plus int");

//                 static_expect<::std::invocable<::std::plus<>, int, bytes>>()
//                     << FORMAT_NS::format("int should be able to plus space size");
//                 static_expect<::std::invocable<::std::plus<>, bytes, bytes>>()
//                     << FORMAT_NS::format("space size should be able plus itself");

//                 static_expect < requires(bytes v)
//                 {
//                     +v;
//                     -v;
//                     ~v;
//                 }
//                 > () << FORMAT_NS::format("space size should have unary operator");

//                 {
//                     constexpr auto v = 1_bit + 1_bit;
//                     static_expect<v == 2_bit>()
//                         << FORMAT_NS::format("1 bit + 1 bit should be 2 bit, actually is {}", v);
//                 }

//                 {
//                     constexpr auto v = 1'000'042_KB;

//                     expect(eq(FORMAT_NS::format("{:-<5MB}", v), "1000MB42KB"sv));
//                     expect(eq(FORMAT_NS::format("{:-<5GB}", v), "1GB0MB42KB"sv));
//                     expect(eq(FORMAT_NS::format("{:-<5.1GB}", v), "1GB--"sv));
//                     expect(eq(FORMAT_NS::format("{:->5.1GB}", v), "--1GB"sv));
//                     expect(eq(FORMAT_NS::format("{:-^5.1GB}", v), "-1GB-"sv));
//                     expect(eq(FORMAT_NS::format("{:.4GB}", v), "1GB0MB42KB0B"sv));
//                     expect(eq(FORMAT_NS::format("{}", 1.2_GB), "1.2GB"sv));
//                 }
//             };
//         };
//         return suite;
//     }
// }
