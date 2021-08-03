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
        template<typename ConditionT, typename Case>
        static constexpr auto get_constexpr_case_pair(Case& c) noexcept
        {
            using type_identity = type_identity<ConditionT>;

            return pair{
                [](const type_identity) noexcept { return invocable<Case, type_identity>; },
                [&c]
                {
                    if constexpr(invocable<Case, type_identity>)
                        return [&c]<typename T>(const T t) noexcept(nothrow_invocable<Case, T>)
                        {
                            invoke(c, t);
                            return true;
                        }; // clang-format off
                    else return [](const type_identity) noexcept { return false; };
                }() // clang-format on
            };
        }
    }

    template<typename ConditionT, typename... Cases>
    constexpr auto constexpr_pattern_match(Cases... cases) noexcept( // clang-format off
    noexcept(
        pattern_match(
            type_identity_v<ConditionT>,
            details::get_constexpr_case_pair<ConditionT>(cases)...
        )
    )
    ) // clang-format on
    {
        return pattern_match(
            type_identity_v<ConditionT>, //
            details::get_constexpr_case_pair<ConditionT>(cases)... //
        );
    }

    template<auto Condition, typename... Cases>
    constexpr auto constexpr_pattern_match(Cases&&... cases)
    {
        return constexpr_pattern_match<constant<Condition>>(std::forward<Cases>(cases)...);
    }
}
