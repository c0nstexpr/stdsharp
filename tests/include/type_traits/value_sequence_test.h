#pragma once
#include "test_utils.h"

namespace stdsharp::test::type_traits
{
    boost::ut::suite& value_sequence_test();

    template<size_t, typename, typename>
    struct insert_by_seq_t_test_params
    {
    };

    template<typename Seq, typename Expect>
    using indexed_t_test_params = stdsharp::type_traits::regular_type_sequence<Seq, Expect>;

    template<typename Seq, typename Expect, typename FrontExpect>
    using append_by_seq_t_test_params =
        stdsharp::type_traits::regular_type_sequence<Seq, Expect, FrontExpect>;

    template<typename TestSeq>
    auto remove_at_by_seq_t_feat() noexcept
    {
        using namespace std;
        using namespace literals;
        using namespace boost::ut;
        using namespace bdd;
        using namespace type_traits;

        return []<typename T, typename Expect>(const indexed_t_test_params<T, Expect>)
        {
            given("given indices sequence") = []
            {
                print(fmt::format("indices type: {}", reflection::type_name<T>()));

                then("use indices type as remove_at_by_seq_t template arg, "
                     "type should be expected") = []
                {
                    print(fmt::format("expected type: {}", reflection::type_name<Expect>()));
                    using actual_t = typename TestSeq::template remove_at_by_seq_t<T>;
                    static_expect<_b(same_as<actual_t, Expect>)>() << //
                        fmt::format("actual type: {}", reflection::type_name<actual_t>());
                };
            };
        };
    }

    template<typename TestSeq>
    auto append_by_seq_t_feat() noexcept
    {
        using namespace std;
        using namespace literals;
        using namespace boost::ut;
        using namespace bdd;
        using namespace type_traits;

        return []<typename Seq, typename Expect, typename FrontExpect>( //
                   const append_by_seq_t_test_params<Seq, Expect, FrontExpect> // clang-format off
        ) // clang-format on
        {
            given("given sequence") = []
            {
                print(fmt::format("sequence type: {}", reflection::type_name<Seq>()));

                then("use seq type as append_by_seq_t template arg, type should be expected") = []
                {
                    print(fmt::format("expected type: {}", reflection::type_name<Expect>()));

                    using actual_t = typename TestSeq::template append_by_seq_t<Seq>;
                    static_expect<same_as<actual_t, Expect>>() << //
                        fmt::format("actual type: {}", reflection::type_name<actual_t>());
                };

                then("use seq type as append_front_by_seq_t template arg, "
                     "type should be expected") = []
                {
                    print(fmt::format("expected type: {}", reflection::type_name<FrontExpect>()));
                    using actual_t = typename TestSeq::template append_front_by_seq_t<Seq>;
                    static_expect<same_as<actual_t, FrontExpect>>() << //
                        fmt::format("actual type: {}", reflection::type_name<actual_t>());
                };
            };
        };
    }

    template<typename TestSeq>
    auto indexed_t_feat() noexcept
    {
        using namespace std;
        using namespace literals;
        using namespace boost::ut;
        using namespace bdd;
        using namespace type_traits;

        return []<typename T, typename Expect>(
                   const indexed_t_test_params<T, Expect>) // clang-format on
        {
            given("given indices sequence") = []
            {
                print(fmt::format("indices type: {}", reflection::type_name<T>()));

                then("use indices type as indexed_by_seq_t template arg, "
                     "type should be expected") = []
                {
                    print(fmt::format("expected type: {}", reflection::type_name<Expect>()));
                    using actual_t = typename TestSeq::template indexed_by_seq_t<T>;
                    static_expect<_b(same_as<actual_t, Expect>)>() << //
                        fmt::format("actual type: {}", reflection::type_name<actual_t>());
                };
            };
        };
    }

    template<typename TestSeq>
    auto invoke_feat() noexcept
    {
        using namespace std;
        using namespace literals;
        using namespace boost::ut;
        using namespace bdd;
        using namespace type_traits;

        return []<typename T>(const T)
        {
            given("given function") = []
            {
                print(fmt::format("function type: {}", reflection::type_name<T>()));

                then("sequence invoke should be invocable") = []
                {
                    static_expect<invocable<decltype(TestSeq::invoke), T>>(); //
                };
            };
        };
    }

    template<typename EmptySeq, typename TestSeq>
    auto construct_feat() noexcept
    {
        using namespace std;
        using namespace literals;
        using namespace boost::ut;
        using namespace bdd;
        using namespace type_traits;

        feature("construct") = []
        {
            static_expect<default_initializable<EmptySeq>>();
            static_expect<default_initializable<TestSeq>>();
        };
    }

    template<typename TestSeq>
    auto insert_by_seq_feat() noexcept
    {
        using namespace std;
        using namespace literals;
        using namespace boost::ut;
        using namespace bdd;
        using namespace type_traits;

        // clang-format off
        return []<size_t Index, typename Seq, typename Expect>(
            const insert_by_seq_t_test_params<Index, Seq, Expect>
        ) // clang-format on
        {
            given("given sequence") = []
            {
                print(fmt::format("sequence type: {}", reflection::type_name<Seq>()));

                then("use seq type as insert_by_seq_t template arg, type should be expected") = []
                {
                    print(fmt::format("expected type: {}", reflection::type_name<Expect>()));
                    using actual_t = typename TestSeq::template insert_by_seq_t<Index, Seq>;
                    static_expect<same_as<actual_t, Expect>>() << //
                        fmt::format("actual type: {}", reflection::type_name<actual_t>());
                };
            }; //
        };
    }
}
