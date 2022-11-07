#pragma once

#include "functional/operations.h"

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
            const Condition& condition,
            ::std::pair<Predicate, Func>... cases //
        ) const
            noexcept((
                (nothrow_predicate<Predicate, Condition> && nothrow_invocable<Func, Condition>)&&...
            ))
        {
            (
                [&condition](::std::pair<Predicate, Func>&& pair)
                {
                    auto&& [first, second] = ::std::move(pair);

                    if(static_cast<bool>(::std::invoke(::std::move(first), condition)))
                    {
                        ::std::invoke(::std::move(second), condition);
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
            template<typename T>
            struct impl
            {
            private:
                template<typename Case>
                static constexpr bool case_nothrow_invocable_ =
                    logical_imply(::std::invocable<Case, T>, nothrow_invocable<Case, T>);

            public:
                template<typename... Cases>
                constexpr void operator()(Cases... cases) const
                    noexcept((case_nothrow_invocable_<Cases> && ...))
                {
                    (
                        []([[maybe_unused]] Cases&& c) noexcept(case_nothrow_invocable_<Cases>)
                        {
                            if constexpr(::std::invocable<Cases, T>)
                            {
                                ::std::invoke(::std::forward<Cases>(c), T{});
                                return true;
                            }
                            else return false;
                        }(::std::move(cases)) ||
                        ...
                    );
                }
            };
        }

        template<typename ConditionT>
        using from_type_fn = details::impl<::std::type_identity<ConditionT>>;

        template<auto Condition>
        using from_constant_fn = details::impl<type_traits::constant<Condition>>;

        template<typename ConditionT>
        inline constexpr from_type_fn<ConditionT> from_type{};

        template<auto Condition>
        inline constexpr from_constant_fn<Condition> from_constant{};
    }
}