#include "functional.h"

namespace blurringshadow::utility
{
    template< //
        typename Condition,
        std::predicate<const Condition>... Predicate,
        std::invocable<const Condition>... Func // clang-format off
    > // clang-format on
    constexpr auto pattern_match(
        const Condition& condition, //
        std::pair<Predicate, Func>... cases // clang-format off
    ) noexcept(((nothrow_predicate<Predicate, Condition> && nothrow_invocable<Func>)&&...))
    {
        using namespace std;
        ( // clang-format on
            [&condition]<typename P>(P& pair)
            {
                if(invoke_r<bool>(pair.first, condition))
                {
                    invoke(pair.second, condition);
                    return true;
                }
                return false;
            }(cases) ||
            ... //
        );
    }

    namespace details
    {
        template<typename ConditionT>
        struct constexpr_pattern_match
        {
            template<typename Case>
            struct case_
            {
                Case c;

                constexpr bool operator()(const auto) noexcept
                {
                    return invocable<type_identity<ConditionT>>;
                }

                constexpr void operator()(const auto) //
                    noexcept(nothrow_invocable<Case, type_identity<ConditionT>>) //
                    requires invocable<type_identity<ConditionT>>
                {
                    invoke(c, type_identity<ConditionT>{});
                }

                constexpr void operator()(const auto) noexcept
                {
                    return [](const auto) noexcept {};
                }
            };

            template<typename Case>
            constexpr auto get_case_pair(Case& c) noexcept
            {
                return pair{case_{reference_wrapper{c}}, case_{reference_wrapper{c}}};
            }
        };
    }

    template<typename ConditionT, typename... Cases>
    constexpr auto constexpr_pattern_match(Cases... cases) noexcept( // clang-format off
        noexcept( 
            pattern_match(
                empty{},
                details::constexpr_pattern_match<ConditionT>::get_case_pair(cases)...
            ) 
        ) 
    ) // clang-format on
    {
        return pattern_match(
            empty{}, //
            details::constexpr_pattern_match<ConditionT>::get_case_pair(cases)... //
        );
    }

    template<auto Condition, typename... Cases>
    constexpr auto constexpr_pattern_match(Cases&&... cases)
    {
        return constexpr_pattern_match<constant<Condition>>(std::forward<Cases>(cases)...);
    }
}
