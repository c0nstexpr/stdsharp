#pragma once

#include "functional.h"

namespace blurringshadow::utility
{
    template< //
        typename Condition,
        ::std::predicate<const Condition>... Predicate,
        ::std::invocable<const Condition>... Func // clang-format off
    > // clang-format on
    constexpr auto pattern_match(
        const Condition& condition, //
        ::std::pair<Predicate, Func>... cases // clang-format off
    ) noexcept(((::blurringshadow::utility::
        nothrow_predicate<Predicate, Condition> && ::blurringshadow::utility::nothrow_invocable<Func>) && ...))
    {
        using namespace std;
        ( // clang-format on
            [&condition]<typename P>(P& pair)
            {
                if(::blurringshadow::utility::invoke_r<bool>(pair.first, condition))
                {
                    ::std::invoke(pair.second, condition);
                    return true;
                }
                return false;
            }(cases) ||
            ... //
        );
    }

    namespace details
    {
        template<typename ConditionT, typename Case>
        static constexpr auto get_constexpr_case_pair(Case& c) noexcept
        {
            using type_identity = ::std::type_identity<ConditionT>;

            return ::std::pair{
                [](const type_identity) noexcept { return ::std::invocable<Case, type_identity>; },
                [&c]
                {
                    if constexpr(::std::invocable<Case, type_identity>)
                        return [&c]<typename T>(const T t) noexcept(
                                   ::blurringshadow::utility::nothrow_invocable<Case, T>)
                        {
                            ::std::invoke(c, t);
                            return true;
                        }; // clang-format off
                    else return [](const type_identity) noexcept { return false; };
                }() // clang-format on
            };
        }
    }

    namespace constexpr_pattern_match
    {
        template<typename ConditionT>
        inline constexpr auto from_type = [](auto... cases) // clang-format off
            noexcept(
                noexcept(
                    ::blurringshadow::utility::pattern_match(
                        ::blurringshadow::utility::type_identity_v<ConditionT>,
                        ::blurringshadow::utility::details::get_constexpr_case_pair<ConditionT>(cases)...
                    )
                )
            ) // clang-format on
        {
            return ::blurringshadow::utility::pattern_match(
                ::blurringshadow::utility::type_identity_v<ConditionT>, //
                ::blurringshadow::utility::details::get_constexpr_case_pair<ConditionT>(cases)... //
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
