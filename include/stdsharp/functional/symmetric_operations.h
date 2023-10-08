#pragma once

#include "bind_lvalue.h"
#include "operations.h"

namespace stdsharp::cpo::inline cpo_impl
{
    namespace details
    {
        template<typename>
        struct arithmetic_operation;

        template<>
        struct arithmetic_operation<plus_assign> : std::type_identity<minus_assign>
        {
        };

        template<>
        struct arithmetic_operation<minus_assign> : std::type_identity<plus_assign>
        {
        };

        template<>
        struct arithmetic_operation<multiplies_assign> : std::type_identity<divides_assign>
        {
        };

        template<>
        struct arithmetic_operation<divides_assign> : std::type_identity<multiplies_assign>
        {
        };

        template<>
        struct arithmetic_operation<negate_assign> : std::type_identity<negate_assign>
        {
        };

        template<>
        struct arithmetic_operation<logical_not_assign> : std::type_identity<logical_not_assign>
        {
        };

        struct specialized_operation_fn
        {
            template<
                typename... Args,
                std::invocable<Args...> Operation,
                typename SymOp = ::meta::_t<arithmetic_operation<Operation>> // clang-format off
            > // clang-format on
            [[nodiscard]] constexpr auto operator()(const Operation, Args&&... args) const
                noexcept(noexcept(bind_lvalue(SymOp{}, cpp_forward(args)...)))
            {
                return bind_lvalue(SymOp{}, cpp_forward(args)...);
            }
        };

        template<typename... T>
        void symmetric_operation(T&&...) = delete;

        struct adl_symmetric_operation_fn
        {
            template<typename... Args>
                requires requires { symmetric_operation(std::declval<Args>()...); }
            [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const
                noexcept(symmetric_operation(std::declval<Args>()...))
            {
                return symmetric_operation(cpp_forward(args)...);
            }
        };

        using symmetric_operation_fn =
            sequenced_invocables<adl_symmetric_operation_fn, specialized_operation_fn>;
    }

    using details::symmetric_operation_fn;

    inline constexpr symmetric_operation_fn symmetric_operation{};
}