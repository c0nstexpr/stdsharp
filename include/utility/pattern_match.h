#pragma once

#include "functional.h"

namespace stdsharp::utility
{
    inline constexpr struct
    {
        template< //
            typename Condition,
            ::std::predicate<const Condition>... Predicate,
            ::std::invocable<const Condition>... Func // clang-format off
        >
        constexpr void operator()( // clang-format on
            const Condition& condition, //
            ::std::pair<Predicate, Func>... cases // clang-format off
        ) const noexcept(((::stdsharp::utility::
            nothrow_predicate<Predicate, Condition> && ::stdsharp::utility::nothrow_invocable<Func>) && ...))
        {
            ( // clang-format on
                [&condition]<typename P>(P&& pair)
                {
                    if(::stdsharp::utility::invoke_r<bool>(::std::move(pair.first), condition))
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
                static constexpr bool case_nothrow_invocable =
                    !::std::invocable<Case, from_type_fn::condition_type_identity> ||
                    ::stdsharp::utility::
                        nothrow_invocable<Case, from_type_fn::condition_type_identity>;

            public:
                template<typename... Cases>
                constexpr auto operator()(Cases... cases) const
                    noexcept((from_type_fn::case_nothrow_invocable<Cases> && ...))
                {
                    (
                        []<typename Case>(Case&& c) // clang-format off
                            noexcept(from_type_fn::case_nothrow_invocable<Case>) // clang-format on
                        {
                            if constexpr(::std::invocable<Case, condition_type_identity>)
                            {
                                ::std::invoke(::std::forward<Case>(c), condition_type_identity{});
                                return true;
                            } // clang-format off
                            else return false;
                        }(::std::move(cases)) || ... // clang-format on
                    );
                }
            };
        }

        template<typename ConditionT>
        inline constexpr ::stdsharp::utility::constexpr_pattern_match::details:: //
            from_type_fn<ConditionT>
                from_type{};

        template<auto Condition>
        inline constexpr auto from_constant = []<typename... Cases>(Cases&&... cases) //
            noexcept(noexcept(::stdsharp::utility::constexpr_pattern_match:: //
                              from_type<::stdsharp::utility:: //
                                        constant<Condition>>(::std::forward<Cases>(cases)...)))
        {
            return ::stdsharp::utility::constexpr_pattern_match:: //
                from_type<::stdsharp::utility:: //
                          constant<Condition>>(::std::forward<Cases>(cases)...);
        };
    }
}
