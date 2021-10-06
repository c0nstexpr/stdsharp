#pragma once

#include "functional/functional.h"

namespace stdsharp
{
    inline constexpr struct
    {
        template<
            typename Condition,
            ::std::predicate<const Condition>... Predicate,
            ::std::invocable<const Condition>... Func // clang-format off
        > // clang-format on
        constexpr void operator()(
            const Condition& condition, //
            ::std::pair<Predicate, Func>... cases // clang-format off
        ) const noexcept(((::stdsharp::concepts::
            nothrow_predicate<Predicate, Condition> && ::stdsharp::concepts::nothrow_invocable<Func>) && ...))
        {
            ( // clang-format on
                [&condition](::std::pair<Predicate, Func>&& pair)
                {
                    if(::stdsharp::functional::invoke_r<bool>(::std::move(pair.first), condition))
                    {
                        ::std::invoke(::std::move(pair.second), condition);
                        return true;
                    }
                    return false;
                }(::std::move(cases)) ||
                ... //
            );
        }
    } pattern_match{};

    namespace constexpr_pattern_match
    {
        namespace details
        {
            template<typename ConditionT>
            struct from_type_fn
            {
            private:
                using condition_type_identity = ::std::type_identity<ConditionT>;

                template<typename Case>
                static constexpr bool case_nothrow_invocable_ =
                    !::std::invocable<Case, from_type_fn::condition_type_identity> ||
                    ::stdsharp::concepts::
                        nothrow_invocable<Case, from_type_fn::condition_type_identity>;

            public:
                template<typename... Cases>
                constexpr auto operator()(Cases... cases) const
                    noexcept((from_type_fn::case_nothrow_invocable_<Cases> && ...))
                {
                    (
                        [](Cases&& c) // clang-format off
                            noexcept(from_type_fn::case_nothrow_invocable_<Cases>) // clang-format on
                        {
                            if constexpr(::std::invocable<Cases, condition_type_identity>)
                            {
                                ::std::invoke(::std::forward<Cases>(c), condition_type_identity{});
                                return true;
                            } // clang-format off
                            else return false;
                        }(::std::move(cases)) || ... // clang-format on
                    );
                }
            };
        }

        template<typename ConditionT>
        inline constexpr ::stdsharp::constexpr_pattern_match::details::from_type_fn<ConditionT>
            from_type{};

        template<auto Condition>
        inline constexpr auto from_constant = ::std::bind_front( //
            ::stdsharp::constexpr_pattern_match:: //
            from_type<::stdsharp::type_traits::constant<Condition>> //
        );
    }
}
