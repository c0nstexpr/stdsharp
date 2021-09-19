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

    template<typename Expect, typename T, template<typename> typename AtBySeq>
    void by_seq_t_then(const std::string_view& then_str)
    {
        using namespace boost::ut;
        using namespace bdd;

        then(then_str) = []
        {
            print(fmt::format("expected type: {}", reflection::type_name<Expect>()));
            using actual_t = AtBySeq<T>;
            static_expect<_b(std::same_as<actual_t, Expect>)>() << //
                fmt::format("actual type: {}", reflection::type_name<actual_t>());
        };
    }
    template<typename T, typename Expect, template<typename> typename AtBySeq>
    auto by_seq_t_feat(const std::string_view given_str, const std::string_view then_str)
    {
        using namespace boost::ut;
        using namespace bdd;

        given(given_str) = [then_str]
        {
            print(fmt::format("sequence type: {}", reflection::type_name<T>()));
            by_seq_t_then<Expect, T, AtBySeq>(then_str);
        };
    }

    template<typename TestSeq>
    constexpr auto remove_at_by_seq_t_feat() noexcept
    {
        return []<typename T, typename Expect>(const indexed_t_test_params<T, Expect>)
        {
            by_seq_t_feat<T, Expect, TestSeq::template remove_at_by_seq_t>(
                "given indices sequence",
                "use indices type as remove_at_by_seq_t template arg, type should be expected" //
            );
        };
    }

    template<typename TestSeq>
    constexpr auto append_by_seq_t_feat() noexcept
    {
        using namespace boost::ut;
        using namespace bdd;

        return []<typename Seq, typename Expect, typename FrontExpect>( // clang-format off
            const append_by_seq_t_test_params<Seq, Expect, FrontExpect>
        ) // clang-format on
        {
            given("given sequence") = []
            {
                print(fmt::format("sequence type: {}", reflection::type_name<Seq>()));

                by_seq_t_then<Expect, Seq, TestSeq::template append_by_seq_t>(
                    "use seq type as append_by_seq_t template arg, type should be expected" //
                );

                by_seq_t_then<FrontExpect, Seq, TestSeq::template append_front_by_seq_t>(
                    "use seq type as append_front_by_seq_t template arg, type should be expected" //
                );
            };
        };
    }

    template<typename TestSeq>
    constexpr auto indexed_t_feat() noexcept
    {
        return []<typename T, typename Expect>(const indexed_t_test_params<T, Expect>)
        {
            by_seq_t_feat<T, Expect, TestSeq::template remove_at_by_seq_t>(
                "given indices sequence",
                "use indices type as indexed_by_seq_t template arg, type should be expected" //
            );
        };
    }

    template<typename TestSeq>
    constexpr auto invoke_feat() noexcept
    {
        using namespace std;
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
    constexpr auto construct_feat() noexcept
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
    constexpr auto insert_by_seq_feat() noexcept
    {
        using namespace std;
        using namespace boost::ut;
        using namespace bdd;

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
            };
        };
    }
}
