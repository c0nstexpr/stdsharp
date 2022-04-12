#pragma once
#include "concepts/concepts.h"
#include "type_traits/core_traits.h"

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
        ) const noexcept(((concepts::nothrow_predicate<Predicate, Condition> &&
            concepts::nothrow_invocable<Func, Condition>) && ...)) // clang-format on
        {
            (
                [&condition](::std::pair<Predicate, Func>&& pair)
                {
                    if(static_cast<bool>(::std::invoke(::std::move(pair.first), condition)))
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
        struct from_type_fn
        {
        private:
            using condition_type_identity = ::std::type_identity<ConditionT>;

            template<typename Case>
            static constexpr bool case_nothrow_invocable_ =
                !::std::invocable<Case, condition_type_identity> ||
                concepts::nothrow_invocable<Case, condition_type_identity>;

        public:
            template<typename... Cases>
            constexpr void operator()(Cases... cases) const
                noexcept((case_nothrow_invocable_<Cases> && ...))
            {
                (
                    []([[maybe_unused]] Cases&& c) // clang-format off
                            noexcept(case_nothrow_invocable_<Cases>) // clang-format on
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

        template<auto Condition>
        using from_constant_fn = from_type_fn<type_traits::constant<Condition>>;

        template<typename ConditionT>
        inline constexpr from_type_fn<ConditionT> from_type{};

        template<auto Condition>
        inline constexpr from_type_fn<type_traits::constant<Condition>> from_constant{};
    }
}
