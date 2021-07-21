#include "type_traits.h"

namespace blurringshadow::utility
{
    /*
    constexpr auto pattern_match = []< // clang-format off
        typename Condition,
        template<typename, typename> typename Pair,
        std::predicate<Condition>... Predicate,
        std::invocable... Func
    >(const Condition& condition, Pair<Predicate, Func>&&... patterns) // clang-format on
        noexcept(((nothrow_predicate<Predicate, Condition> && nothrow_invocable<Func>)&&...)) //
        requires(
            std::same_as<std::remove_cvref_t<Pair<Predicate, Func>>, std::pair<Predicate, Func>> &&
            ...)
    {
        (
            []<typename P>(P&& pair)
            {
                auto&& [predicate, func] = std::forward<P>(pair);
                if(predicate(condition)) std::invoke(std::forward<decltype(func)>(func));
            }(patterns),
            ...);
    };

    template<typename Func>
    struct pattern
    {
        Func func;

        template<auto Condition>
        constexpr decltype(auto) operator()() noexcept(noexcept(func(constant<Condition>{}))) //
            requires std::invocable<Func, constant<Condition>>
        {
            return func(constant<Condition>{});
        }
    };

    inline constexpr auto constexpr_pattern_match = []<auto Condition, typename... Patterns>(
        Patterns && ... patterns //
    )
    {
        []<typename P>()
        {
            if constexpr(std::invocable<P>) p();
        }();
    };
    */
}