#pragma once

#include "functional.h"

namespace blurringshadow::utility
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
        ) const noexcept(((::blurringshadow::utility::
            nothrow_predicate<Predicate, Condition> && ::blurringshadow::utility::nothrow_invocable<Func>) && ...))
        {
            ( // clang-format on
                [&condition]<typename P>(P&& pair)
                {
                    if(::blurringshadow::utility::invoke_r<bool>(
                           ::std::move(pair.first), condition))
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
        template<typename ConditionT>
        inline constexpr auto from_type = []<typename... Cases>(Cases... cases) // clang-format off
            noexcept(
                (
                    (
                        !::std::invocable<Cases, ::std::type_identity<ConditionT>> ||
                        ::blurringshadow::utility::nothrow_invocable<Cases, ::std::type_identity<ConditionT>>
                    ) && ...
                )
            ) // clang-format on
        {
            using condition_type_identity = ::std::type_identity<ConditionT>;

            (
                []<typename Case>(Case&& c) noexcept(
                    !::std::invocable<Case, condition_type_identity> ||
                    ::blurringshadow::utility::nothrow_invocable<Case,  condition_type_identity> //
                )
                {

                    if constexpr(::std::invocable<Case, condition_type_identity>)
                    {
                        ::std::invoke(::std::forward<Case>(c), condition_type_identity{});
                        return true;
                    } // clang-format off
                    else return false;
                }(::std::move(cases)) || ... // clang-format on
            );
        };

        template<auto Condition>
        inline constexpr auto from_constant = []<typename... Cases>(Cases&&... cases) //
            noexcept(noexcept(::blurringshadow::utility::constexpr_pattern_match:: //
                              from_type<::blurringshadow::utility:: //
                                        constant<Condition>>(::std::forward<Cases>(cases)...)))
        {
            return ::blurringshadow::utility::constexpr_pattern_match:: //
                from_type<::blurringshadow::utility:: //
                          constant<Condition>>(::std::forward<Cases>(cases)...);
        };
    }
}
