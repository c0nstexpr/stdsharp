#pragma once

#include "bind.h"

namespace stdsharp::functional
{
    namespace details
    {
        template<typename>
        struct arithmetic_operation;

        template<>
        struct arithmetic_operation<functional::plus_assign> :
            ::std::type_identity<functional::minus_assign>
        {
        };

        template<>
        struct arithmetic_operation<functional::minus_assign> :
            ::std::type_identity<functional::plus_assign>
        {
        };

        template<>
        struct arithmetic_operation<functional::multiplies_assign> :
            ::std::type_identity<functional::divides_assign>
        {
        };

        template<>
        struct arithmetic_operation<functional::divides_assign> :
            ::std::type_identity<functional::multiplies_assign>
        {
        };

        template<>
        struct arithmetic_operation<functional::negate_assign> :
            ::std::type_identity<functional::negate_assign>
        {
        };

        template<>
        struct arithmetic_operation<functional::logical_not_assign> :
            ::std::type_identity<functional::logical_not_assign>
        {
        };

        struct specialized_operation_fn
        {
            template<
                typename... Args,
                ::std::invocable<Args...> Operation,
                typename SymOp = typename arithmetic_operation<Operation>::type // clang-format off
            > // clang-format on
            [[nodiscard]] constexpr auto operator()(const Operation, Args&&... args) const
                noexcept(noexcept(bind(SymOp{}, ::std::forward<Args>(args)...)))
            {
                return bind(SymOp{}, ::std::forward<Args>(args)...);
            }
        };

        template<typename... T>
        void symmetric_operation(T&&...) = delete;

        struct adl_symmetric_operation_fn
        {
            template<typename... Args>
                requires requires { symmetric_operation(::std::declval<Args>()...); }
            [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const
                noexcept(symmetric_operation(::std::declval<Args>()...))
            {
                return symmetric_operation(::std::forward<Args>(args)...);
            }
        };

        using symmetric_operation_fn =
            sequenced_invocables<adl_symmetric_operation_fn, specialized_operation_fn>;
    }

    inline namespace cpo
    {
        using details::symmetric_operation_fn;

        inline constexpr symmetric_operation_fn symmetric_operation{};
    }
}