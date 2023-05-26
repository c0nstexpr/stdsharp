#pragma once

#include "functional/operations.h"

namespace stdsharp
{
    inline constexpr struct
    {
        template<
            typename Condition,
            std::predicate<const Condition&>... Predicate,
            std::invocable<const Condition&>... Func // clang-format off
        > // clang-format on
        constexpr void operator()(
            const Condition& condition,
            std::pair<Predicate, Func>... cases //
        ) const
            noexcept((
                (nothrow_predicate<Predicate, Condition> && nothrow_invocable<Func, Condition>)&&...
            ))
        {
            (
                [&condition](std::pair<Predicate, Func>&& pair)
                {
                    auto&& [first, second] = cpp_move(pair);

                    if(std::invoke(cpp_move(first), condition))
                    {
                        std::invoke(cpp_move(second), condition);
                        return true;
                    }
                    return false;
                }(cpp_move(cases)) ||
                ... //
            );
        }

        template<
            typename Condition,
            std::invocable<const Condition>... Func // clang-format off
        > // clang-format on
        constexpr void operator()(
            const Condition& condition,
            std::pair<Condition, Func>... cases //
        ) const noexcept((nothrow_invocable<Func, Condition> && ...))
        {
            (*this)(
                condition,
                make_pair(
                    std::bind_front(equal_to_v, cpp_move(cases.first)),
                    cpp_move(cases.second)
                )...
            );
        }
    } pattern_match{};

    namespace constexpr_pattern_match
    {
        namespace details
        {
            template<typename T>
            struct impl
            {
            private:
                template<typename Case>
                static constexpr bool case_nothrow_invocable_ =
                    logical_imply(std::invocable<Case, T>, nothrow_invocable<Case, T>);

            public:
                template<typename... Cases>
                constexpr void operator()(Cases... cases) const
                    noexcept((case_nothrow_invocable_<Cases> && ...))
                {
                    (
                        []([[maybe_unused]] Cases&& c) noexcept(case_nothrow_invocable_<Cases>)
                        {
                            if constexpr(std::invocable<Cases, T>)
                            {
                                std::invoke(cpp_forward(c), T{});
                                return true;
                            }
                            else return false;
                        }(cpp_move(cases)) ||
                        ...
                    );
                }
            };
        }

        template<typename ConditionT>
        using from_type_fn = details::impl<std::type_identity<ConditionT>>;

        template<auto Condition>
        using from_constant_fn = details::impl<constant<Condition>>;

        template<typename ConditionT>
        inline constexpr from_type_fn<ConditionT> from_type{};

        template<auto Condition>
        inline constexpr from_constant_fn<Condition> from_constant{};
    }
}