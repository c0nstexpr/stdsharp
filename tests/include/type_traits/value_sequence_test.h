#pragma once

#include "test.h"

using namespace std;

template<typename Expect, typename T, template<typename> typename AtBySeq>
void by_seq_t_then(const std::string_view& then_str)
{
    STATIC_REQUIRE(same_as<AtBySeq<T>, Expect>);
}

template<typename T, typename Expect, template<typename> typename AtBySeq>
auto by_seq_t_feat(
    const std::string_view given_str,
    const std::string_view then_str,
    const boost::ut::reflection::source_location& sl //
)
{
    using namespace std;
    using namespace boost::ut;
    using namespace bdd;

    struct fn
    {
        const boost::ut::reflection::source_location sl;
        const std::string_view then_str;

        auto operator()() const
        {
            print(fmt::format("sequence type: {}", reflection::type_name<T>()));
            by_seq_t_then<Expect, T, AtBySeq>(then_str, sl);
        }
    };

    given(given_str) = fn{sl, then_str};
} // clang-format on

template<typename TestSeq>
constexpr auto remove_at_by_seq_t_feat(
    const boost::ut::reflection::source_location& sl =
        boost::ut::reflection::source_location::current() // clang-format off
    ) noexcept // clang-format on
{
    return std::bind_front(
        []<typename T, typename Expect>(
            const boost::ut::reflection::source_location& sl,
            const indexed_by_seq_t_test_params<T, Expect> //
        )
        {
            by_seq_t_feat<T, Expect, TestSeq::template remove_at_by_seq_t>(
                "given indices sequence",
                "use indices type as remove_at_by_seq_t template arg, type should be "
                "expected",
                sl //
            );
        },
        sl //
    );
}

template<typename TestSeq>
constexpr auto append_by_seq_t_feat( //
    const boost::ut::reflection::source_location& sl =
        boost::ut::reflection::source_location::current() // clang-format off
    ) noexcept // clang-format on
{
    using namespace std;
    using namespace boost::ut;
    using namespace bdd;

    return bind_front(
        []<typename Seq, typename Expect, typename FrontExpect>(
            const boost::ut::reflection::source_location& sl,
            const append_by_seq_t_test_params<Seq, Expect, FrontExpect> //
        )
        {
            struct fn
            {
                const boost::ut::reflection::source_location sl;
                auto operator()() const
                {
                    print(fmt::format("sequence type: {}", reflection::type_name<Seq>()));

                    by_seq_t_then<Expect, Seq, TestSeq::template append_by_seq_t>(
                        "use seq type as append_by_seq_t template arg, type should be "
                        "expected",
                        sl //
                    );

                    by_seq_t_then<FrontExpect, Seq, TestSeq::template append_front_by_seq_t>(
                        "use seq type as append_front_by_seq_t template arg, type should "
                        "be "
                        "expected",
                        sl //
                    );
                }
            };

            given("given sequence") = fn{sl};
        },
        sl //
    );
}

template<typename TestSeq>
constexpr auto indexed_by_seq_t_feat( //
    const boost::ut::reflection::source_location& sl =
        boost::ut::reflection::source_location::current() // clang-format off
    ) noexcept // clang-format on
{
    return std::bind_front(
        []<typename T, typename Expect>(
            const boost::ut::reflection::source_location& sl,
            const indexed_by_seq_t_test_params<T, Expect> //
        )
        {
            by_seq_t_feat<T, Expect, TestSeq::template indexed_by_seq_t>(
                "given indices sequence",
                "use indices type as indexed_by_seq_t template arg, type should be "
                "expected",
                sl //
            );
        },
        sl //
    );
}

template<typename TestSeq>
constexpr auto invoke_feat( //
    const boost::ut::reflection::source_location& sl =
        boost::ut::reflection::source_location::current() // clang-format off
    ) noexcept // clang-format on
{
    using namespace std;
    using namespace boost::ut;
    using namespace bdd;

    return bind_front(
        []<typename T>(const boost::ut::reflection::source_location& sl, const T)
        {
            given("given function") = bind_front(
                [](const boost::ut::reflection::source_location& sl)
                {
                    print(fmt::format("function type: {}", reflection::type_name<T>()));

                    then("sequence invoke should be invocable") = bind_front(
                        &static_expect< //
                            invocable<
                                typename TestSeq:: // clang-format off
                                        template invoke_fn<stdsharp::type_traits::empty_t>,
                                    T
                                >
                            >,
                            sl // clang-format on
                    );
                },
                sl //
            );
        },
        sl //
    );
}

template<typename EmptySeq, typename TestSeq>
constexpr auto construct_feat( //
    const boost::ut::reflection::source_location& sl =
        boost::ut::reflection::source_location::current() // clang-format off
    ) noexcept // clang-format on
{
    using namespace std;
    using namespace boost::ut;
    using namespace bdd;

    struct fn
    {
        const boost::ut::reflection::source_location sl;

        auto operator()() const
        {
            static_expect<default_initializable<EmptySeq>>(sl);
            static_expect<default_initializable<TestSeq>>(sl);
        }
    };

    feature("construct") = fn{sl};
}

template<typename TestSeq>
constexpr auto insert_by_seq_feat( //
    const boost::ut::reflection::source_location& sl =
        boost::ut::reflection::source_location::current() // clang-format off
    ) noexcept // clang-format on
{
    using namespace std;
    using namespace boost::ut;
    using namespace bdd;

    // clang-format off
        return bind_front(
            []<size_t Index, typename Seq, typename Expect>(
                const boost::ut::reflection::source_location& sl,
                const insert_by_seq_t_test_params<Index, Seq, Expect>
            ) // clang-format on
        {
            struct fn
            {
                const boost::ut::reflection::source_location sl;

                auto operator()() const
                {
                    print(fmt::format("sequence type: {}", reflection::type_name<Seq>()));

                    struct inner_fn
                    {
                        const boost::ut::reflection::source_location sl;

                        auto operator()() const
                        {
                            print(
                                fmt::format("expected type: {}", reflection::type_name<Expect>()));
                            using actual_t = typename TestSeq::template insert_by_seq_t<Index, Seq>;
                            static_expect<same_as<actual_t, Expect>>(sl) << //
                                fmt::format("actual type: {}", reflection::type_name<actual_t>());
                        }
                    };

                    then("use seq type as insert_by_seq_t template arg, "
                         "type should be expected") = inner_fn{sl};
                }
            };

            given("given sequence") = fn{sl};
        },
        sl //
    );
}
}